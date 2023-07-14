#extension GL_GOOGLE_include_directive : enable

#ifndef PBR_GLSL
#define PBR_GLSL
#include "global_ubo.glsl"
#include "scene_ssbo.glsl"
#include "pbr_math.glsl"
#include "fixed_precision.glsl"

layout (set = 2, binding = 0) uniform samplerCube environmentPrefilteredMap;
layout (set = 2, binding = 1) uniform samplerCube irradianceMap;
layout (set = 2, binding = 2) uniform sampler2D brdfLUT;

layout (set = 3, binding = 0) uniform sampler2DShadow shadowMapPCF;
layout (set = 3, binding = 1) uniform sampler3D virtual_PropagatedLPV;
layout (set = 3, binding = 2) uniform sampler2D skyLightVFAOmap;
layout (set = 3, binding = 3) uniform sampler2D causticMap;

struct FragmentLitPBRData {
	vec3 position;
	vec3 normal;
	vec3 diffuse;
	float specular;
	float roughness;
	float metallic;
	float ambientOcclusion;
};
struct FragmentClearCoatPBRData {
	float clearCoat;
	float clearCoatRoughness;
};

struct PrivateStaticPBRData {
	float F0; // IOR stuff
	float m; // roughness ^ 2
	float m2; // roughness ^ 4
	
	vec3 V; // eye vector
	vec3 N; // normal vector
	float NdV;
};
struct PrivatePBRData {	
	vec3 H; // half way vector
	vec3 L; // fragment to light vector
	float HdL;
	float NdL;
	float NdH;
	float VdH;
};



struct PrivateLightingData {
	vec3 radiance;
};

float F_Schlick(float F0, float dv){
	float x = 1.0 - dv;
	float x2 = x*x;
	float x4 = x2*x2;
	float Fc = x4*x;
	
    return F0 + (1.0 - F0) * Fc;
}
float F_SchlickRoughness(float F0, float dv, float roughness){
	float x = 1.0 - dv;
	float x2 = x*x;
	float x4 = x2*x2;
	float Fc = x4*x;
	
    return F0 + (max(1.0 - roughness, F0) - F0) * Fc;
}

vec3 pbrSchlickBeckmannBRDF(PrivateStaticPBRData pbrSData, PrivatePBRData pbrData, PrivateLightingData lightingData) {
	float F = F_Schlick(pbrSData.F0, pbrData.HdL);
    float V = GVis_SchlickGGX(pbrSData.NdV, pbrData.NdL, pbrData.NdH, pbrData.VdH, pbrSData.m);
    float D = D_beckmann(pbrData.NdH, pbrSData.m2);

	float brdfResult = pbrData.NdL * F * D * V;
    return brdfResult * lightingData.radiance;
}

vec3 pbrLambertDiffuseBRDF(PrivateStaticPBRData pbrSData, PrivatePBRData pbrData, PrivateLightingData lightingData) {
    return (1.0 / PI) * lightingData.radiance * pbrData.NdL;
}
const vec2 poissonDisk[16] = vec2[]( 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);
float sampleShadow(sampler2DShadow shadow, vec3 world, mat4 vp) {
	const float BIAS = 0.002;
	const int PCF_KERNEL = 1;
	
	vec4 projCoords = vp * vec4(world, 1.0);
	projCoords.xy /= projCoords.w;
	projCoords.xy = projCoords.xy * 0.5 + 0.5;
	float accum = 0.0;
	float samples = 0.0;
	float texelSize = 1.0 / float(textureSize(shadowMapPCF, 0).x);
	for (int x = -PCF_KERNEL; x <= PCF_KERNEL; x++) {
		for (int y = 0; y <= PCF_KERNEL; y++) {
			vec2 off = texelSize * vec2(float(x), float(y));
			accum += texture(shadow, vec3(projCoords.xy + off, projCoords.z - BIAS));
			samples+=1.0;
		}
	}
	float causticAccum = texture(causticMap, projCoords.xy).x;
	
	return 0.5 * accum / samples + causticAccum;
}

float sampleVFAO(sampler2D vfaoMap, vec3 world, mat4 vfaoVP) {
	vec4 projCoords = vfaoVP * vec4(world, 1.0);
	projCoords.xy /= projCoords.w;
	vec2 uv = projCoords.xy * 0.5 + 0.5;
	float currentDepth = projCoords.z;
	
	vec4 variance_fields = texture(vfaoMap, uv).xyzw;
	
	vec2 moments = variance_fields.xy;
	vec2 fields = variance_fields.zw;
	
	float p = step(currentDepth, moments.x);
	float variance = max(moments.y - moments.x * moments.x, 0.00002);
	float d = projCoords.z - moments.x;
	float pMax = variance / (variance + d*d);
	float ddp = fwidth(pMax);
	float ls = smoothstep(max(pow(ddp, 1.0 / 32.0), 0.0), 1.0, pMax);
	float baseOcclusion = min(max(p, ls), 1.0);
	
	const float neighbouringOcclusionPower = 4.0; // higher values are sharper AO values
	
	float df = exp(pow(-1.012 * fields.x, 256.0));
	float visDelta = clamp(log(fields.x + pow(fields.y, neighbouringOcclusionPower)), 0.0, 1.0);
	float diff = (1.0 - abs(projCoords.z - moments.x)) / fields.x;
	diff = 0.0; // temporary
	
	
	return mix(baseOcclusion, visDelta, diff);
}

const float SH_C0 = 0.282094792; // 1 / 2sqrt(pi)
const float SH_C1 = 0.488602512; // sqrt(3/pi) / 2
vec4 dirToSH(vec3 dir) {
	return vec4(SH_C0, -SH_C1 * dir.y, SH_C1 * dir.z, -SH_C1 * dir.x);
}
vec3 worldToUV(vec3 world, vec3 center, vec3 extent) {
	return ( (world - center) / extent + 1.0) / 2.0;
}
vec3 shToIrradiance(vec3 normal, vec4 shR, vec4 shG, vec4 shB) {
	vec4 SHintensity = dirToSH(-normal);
	vec3 lpvIntensity = vec3(
		dot(SHintensity, shR),
		dot(SHintensity, shG),
		dot(SHintensity, shB)
	);
	
	return (1.0 / PI) * max(vec3(0.0), lpvIntensity);
}

const int SAMPLER_MODE_REPEAT = 0;
const int SAMPLER_MODE_CLAMP_EDGE = 1;
const int SAMPLER_MODE_CLAMP_BORDER = 2;

	// shift the sampling away from the border to prevent leaking from nearby textures
vec3 limitSamplerEdge(sampler3D atlas, vec3 uv) {
	vec3 half_Texel = 0.5 / textureSize(atlas, 0).xyz;
	vec3 UV_LIMIT_MAX = 1.0 - half_Texel;
	vec3 UV_LIMIT_MIN = half_Texel;
	return clamp(uv, UV_LIMIT_MIN, UV_LIMIT_MAX);
}
vec4 sampleVirtual3D(sampler3D atlas, in vec3 uv, vec3 minc, vec3 maxc, int samplerMode, vec4 borderColor) {	
	vec3 delta = maxc - minc;
	vec3 textureCoord;
	if (samplerMode == SAMPLER_MODE_REPEAT) {
		uv = limitSamplerEdge(atlas, fract(uv));
		
		textureCoord = mod(uv * delta, delta) + minc;
	} else if (samplerMode == SAMPLER_MODE_CLAMP_EDGE) {
		uv = limitSamplerEdge(atlas, uv);
		
		textureCoord = uv * delta + minc;
	}else if (samplerMode == SAMPLER_MODE_CLAMP_BORDER) {
		if (any(greaterThan(abs(uv * 2.0 - 1.0), vec3(1.0, 1.0, 1.0)))) {
			return borderColor;
		}
		uv = limitSamplerEdge(atlas, uv);
		textureCoord = uv * delta + minc;
	}
	
	return textureLod(atlas, textureCoord, 0.0);
}


vec3 pbrCalculateLighting(FragmentLitPBRData fragment, FragmentClearCoatPBRData fragmentcc) {
	vec3 cameraPosWorld = ubo.invViewMatrix[3].xyz;
	vec3 N = normalize(fragment.normal);
	vec3 V = normalize(cameraPosWorld - fragment.position);
	vec3 I = -V;
	vec3 R = reflect(I, N);
	
	float F0 = 0.04; // refraction index of 1.5 (F0 = (1.0 - n) ^ 2 / (1.0 + n) ^ 2)
	PrivateStaticPBRData pbrSData;
	pbrSData.F0 = F0;
	
	pbrSData.m = fragment.roughness * fragment.roughness; 
	pbrSData.m2 = pbrSData.m * pbrSData.m;
	
	pbrSData.V = V; 
	pbrSData.N = N; 
	pbrSData.NdV = max(dot(N, V), 0.0);
	
	
	PrivateStaticPBRData ccpbrSData = pbrSData;
	ccpbrSData.m = fragmentcc.clearCoatRoughness * fragmentcc.clearCoatRoughness; 
	ccpbrSData.m2 = ccpbrSData.m * ccpbrSData.m;
	
	const float fresnelTotalInternalReflection = F_Schlick(F0, pbrSData.NdV);
	
	bool enableClearCoatBRDF = fragmentcc.clearCoat != 0.0;
	
	vec3 specular = vec3(0.0);
	vec3 clearCoat = vec3(0.0);
	vec3 irradiance = vec3(0.0);
	vec3 diffuse = vec3(0.0);
	
	if (scene.hasLPV) {
		vec3 lpvAccum = vec3(0.0);
		for (int i = 0; i < scene.lpv.cascadeCount; i++) {
			vec3 lpvUV = worldToUV(fragment.position, scene.lpv.center, scene.lpv.cascades[i].extent);
			 lpvAccum += scene.lpv.boost * 100.0 * shToIrradiance(
				fragment.normal,
				sampleVirtual3D(virtual_PropagatedLPV, lpvUV, scene.lpv.cascades[i].virtualPropagatedGridRedUVMin, scene.lpv.cascades[i].virtualPropagatedGridRedUVMax, SAMPLER_MODE_CLAMP_BORDER, vec4(0.0)),
				sampleVirtual3D(virtual_PropagatedLPV, lpvUV, scene.lpv.cascades[i].virtualPropagatedGridGreenUVMin, scene.lpv.cascades[i].virtualPropagatedGridGreenUVMax, SAMPLER_MODE_CLAMP_BORDER, vec4(0.0)),
				sampleVirtual3D(virtual_PropagatedLPV, lpvUV, scene.lpv.cascades[i].virtualPropagatedGridBlueUVMin, scene.lpv.cascades[i].virtualPropagatedGridBlueUVMax, SAMPLER_MODE_CLAMP_BORDER, vec4(0.0))
				
			) ; 
		}
		irradiance += lpvAccum;
	}
	
	if (scene.hasSkyLight) {
		float AO = 1.0; //sampleVFAO(skyLightVFAOmap, fragment.position, scene.skyLight.vfaovp);
		const float maxEnvMapMipLevel = float(textureQueryLevels(environmentPrefilteredMap)) - 1.0;
	
		const vec3 skyLightTint = AO * scene.skyLight.tint.rgb * scene.skyLight.tint.a;
		const vec3 prefilteredColor = skyLightTint * textureLod(environmentPrefilteredMap, R, pbrSData.m * maxEnvMapMipLevel).rgb;
		const vec2 envBRDF = textureLod(brdfLUT, vec2(pbrSData.NdV, fragment.roughness), 0.0).xy;
		specular += prefilteredColor * (F_SchlickRoughness(F0, pbrSData.NdV, fragment.roughness) * envBRDF.x + envBRDF.y);
		if (enableClearCoatBRDF) {
			const vec3 ccprefilteredColor = skyLightTint * textureLod(environmentPrefilteredMap, R, ccpbrSData.m * maxEnvMapMipLevel).rgb;
			const vec2 ccenvBRDF = textureLod(brdfLUT, vec2(pbrSData.NdV, fragmentcc.clearCoatRoughness), 0.0).xy;
			clearCoat += ccprefilteredColor * (F_SchlickRoughness(F0, pbrSData.NdV, fragmentcc.clearCoatRoughness) * ccenvBRDF.x + ccenvBRDF.y);
		}
		irradiance += skyLightTint * textureLod(irradianceMap, R, 0.0).rgb * fragment.diffuse * (1.0 - fragment.metallic);
	}
	
	for (uint i = 0; i < scene.pointLightCount; i++) {
		vec3 fragToLight = scene.pointLights[i].position - fragment.position;
		vec3 L = normalize(fragToLight);
		float attenuation = 1.0 / dot(fragToLight, fragToLight);
		vec3 radiance = attenuation * scene.pointLights[i].color.rgb * scene.pointLights[i].color.a;	
		vec3 H = normalize(V + L);
		
		PrivatePBRData pbrData;
		pbrData.HdL = max(dot(H, L), 0.0);
		pbrData.NdL = max(dot(N, L), 0.0);
		pbrData.NdH = max(dot(N, H), 0.0);
		pbrData.VdH = max(dot(V, H), 0.0);
		
		PrivateLightingData lightingData;
		lightingData.radiance = radiance;
		
		diffuse += (1.0 - F_Schlick(F0, pbrData.NdL)) * pbrLambertDiffuseBRDF(pbrSData, pbrData, lightingData);
		specular += pbrSchlickBeckmannBRDF(pbrSData, pbrData, lightingData);
		if (enableClearCoatBRDF) {
			clearCoat += pbrSchlickBeckmannBRDF(ccpbrSData, pbrData, lightingData);
		}
	}
	
	if (scene.hasDirectionalLight) {
		vec3 L = -scene.directionalLight.direction;
		vec3 radiance = scene.directionalLight.color.rgb * scene.directionalLight.color.a;
		vec3 H = normalize(V + L);
		
		PrivatePBRData pbrData;
		pbrData.HdL = max(dot(H, L), 0.0);
		pbrData.NdL = max(dot(N, L), 0.0);
		pbrData.NdH = max(dot(N, H), 0.0);
		pbrData.VdH = max(dot(V, H), 0.0);
		
		PrivateLightingData lightingData;
		lightingData.radiance = radiance;
		
		float shadow = sampleShadow(shadowMapPCF, fragment.position, scene.directionalLight.vp);
		diffuse += shadow * (1.0 - F_Schlick(F0, pbrData.NdL)) * pbrLambertDiffuseBRDF(pbrSData, pbrData, lightingData);
		diffuse += 
		specular += shadow *  pbrSchlickBeckmannBRDF(pbrSData, pbrData, lightingData);
		if (enableClearCoatBRDF) {
			clearCoat += shadow * pbrSchlickBeckmannBRDF(ccpbrSData, pbrData, lightingData);
		}
	}
	vec3 reflectedIrradiance = irradiance * fragment.diffuse * (1.0 - fragment.metallic);
	vec3 reflectedDiffuse = diffuse * fragment.diffuse * (1.0 - fragment.metallic) * (1.0 - fresnelTotalInternalReflection);
	vec3 reflectedSpecular = mix(specular, specular * fragment.diffuse, fragment.metallic) * fragment.specular;
	vec3 reflectedClearCoat = clearCoat * fragmentcc.clearCoat;
	return reflectedIrradiance + reflectedDiffuse + reflectedSpecular + reflectedClearCoat;
}
#endif