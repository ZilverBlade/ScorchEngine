#extension GL_GOOGLE_include_directive : enable

#ifndef PBR_GLSL
#define PBR_GLSL
#include "global_ubo.glsl"
#include "scene_ssbo.glsl"
#include "constants.glsl"

struct FragmentPBRData {
	vec3 position;
	vec3 normal;
	vec3 diffuse;
	float specular;
	float roughness;
	float metallic;
	float ambientOcclusion;
};

struct PrivatePBRData {
	float F0; // IOR stuff
	float m; // roughness ^ 2
	float m2; // roughness ^ 4
	
	vec3 H; // V + L idfk what this is
	vec3 L; // fragment to light vector
	vec3 V; // eye vector
	vec3 N; // normal
	
	float HdL;
	float NdL;
	float NdV;
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

float D_beckmann(PrivatePBRData pbrData) {
    float NdH2 = pbrData.NdH * pbrData.NdH;
    return exp((NdH2 - 1.0) / max(pbrData.m2 * NdH2, 1e-6)) / max(PI * pbrData.m2 * NdH2 * NdH2, 1e-6);
}

float D_GGX(PrivatePBRData pbrData) {
    float d = (pbrData.NdH * pbrData.m2 - pbrData.NdH) * pbrData.NdH + 1.0;
    return pbrData.m2 / max(PI * d * d, 1e-6);
}
float Vis_Schlick(PrivatePBRData pbrData) {
    float x = (pbrData.m + 1.0);
    float k = (x * x) / 8.0;
	
	float Vis_SchlickV = pbrData.NdV * (1.0 - k) + k;
	float Vis_SchlickL = pbrData.NdL * (1.0 - k) + k;
	return 0.25 / max(Vis_SchlickV * Vis_SchlickL, 1e-6);
}
                   
vec3 pbrSchlickBeckmannBRDF(PrivatePBRData pbrData, PrivateLightingData lightingData) {
	float F = F_Schlick(pbrData.F0, pbrData.HdL);
    float V = Vis_Schlick(pbrData);
    float D = D_beckmann(pbrData);

	float brdfResult = pbrData.NdL * F * D * V;
    return brdfResult * lightingData.radiance;
}

vec3 pbrLambertDiffuseBRDF(PrivatePBRData pbrData, PrivateLightingData lightingData) {
    return (1.0 / PI) * lightingData.radiance * pbrData.NdL;
}

vec3 pbrCalculateLighting(FragmentPBRData fragment) {
	vec3 cameraPosWorld = ubo.invViewMatrix[3].xyz;
	vec3 N = normalize(fragment.normal);
	vec3 V = normalize(cameraPosWorld - fragment.position);
	vec3 I = normalize(fragment.position - cameraPosWorld);
	vec3 R = reflect(I, N);
	
	float F0 = 0.04; // refraction index of 1.5
	PrivatePBRData pbrDataBase;
	pbrDataBase.F0 = F0;
	
	pbrDataBase.m = fragment.roughness * fragment.roughness; 
	pbrDataBase.m2 = pbrDataBase.m * pbrDataBase.m;
	
	pbrDataBase.V = V; 
	pbrDataBase.N = N; 
	pbrDataBase.NdV = max(dot(N, V), 0.0);
	
	vec3 diffuse = vec3(0.0);
	vec3 specular = vec3(0.0);
	
	for (uint i = 0; i < scene.pointLightCount; i++) {
		vec3 fragToLight = scene.pointLights[i].position - fragment.position;
		vec3 L = normalize(fragToLight);
		float attenuation = 1.0 / dot(L, L);
		vec3 radiance = attenuation * scene.pointLights[i].color.rgb * scene.pointLights[i].color.a;	
		vec3 H = normalize(V + L);
		
		PrivatePBRData pbrData = pbrDataBase;
		pbrData.HdL = max(dot(H, L), 0.0);
		pbrData.NdL = max(dot(N, L), 0.0);
		pbrData.NdH = max(dot(N, H), 0.0);
		pbrData.VdH = max(dot(V, H), 0.0);
		
		PrivateLightingData lightingData;
		lightingData.radiance = radiance;
		
		diffuse += (1.0 - F_Schlick(F0, pbrData.NdL)) * (1.0 - F_Schlick(F0, pbrData.NdV)) * pbrLambertDiffuseBRDF(pbrData, lightingData);
		specular += pbrSchlickBeckmannBRDF(pbrData, lightingData);
	}
	
	for (uint i = 0; i < scene.directionalLightCount; i++) {
		vec3 L = -scene.directionalLights[i].direction;
		vec3 radiance = scene.directionalLights[i].color.rgb * scene.directionalLights[i].color.a;
		vec3 H = normalize(V + L);
		
		PrivatePBRData pbrData = pbrDataBase;
		pbrData.HdL = max(dot(H, L), 0.0);
		pbrData.NdL = max(dot(N, L), 0.0);
		pbrData.NdH = max(dot(N, H), 0.0);
		pbrData.VdH = max(dot(V, H), 0.0);
		
		PrivateLightingData lightingData;
		lightingData.radiance = radiance;
		
		diffuse += (1.0 - F_Schlick(F0, pbrData.NdL)) * (1.0 - F_Schlick(F0, pbrData.NdV)) * pbrLambertDiffuseBRDF(pbrData, lightingData);
		specular += pbrSchlickBeckmannBRDF(pbrData, lightingData);
	}
	vec3 reflectedSpecular = mix(specular, specular * fragment.diffuse, fragment.metallic) * fragment.specular;
	vec3 reflectedDiffuse = mix(diffuse * fragment.diffuse, vec3(0.0), fragment.metallic);
	return reflectedDiffuse + reflectedSpecular;
}
#endif