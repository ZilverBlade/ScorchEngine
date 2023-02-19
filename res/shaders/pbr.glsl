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
	float m; // roughness ^ 2
	float m2; // roughness ^ 4
	
	vec3 H; // V + L idfk what this is
	vec3 L; // fragment to light vector
	vec3 V; // eye vector
	vec3 N; // normal
	
	float NdL;
	float NdV;
	float NdH;
	float VdH;
};

struct PrivateLightingData{
	vec3 radiance;
	vec3 fresnel;
};

float pbrLambertianDiffuse() {
	return (1.0 / PI);
}

vec3 F_Schlick(vec3 F0, PrivatePBRData pbrData){
	float x = 1.0 - pbrData.VdH;
	float x2 = x*x;
	float x4 = x2*x2;
	float Fc = x4*x;
	
    return F0 + (1.0 - F0) * Fc;
}

float D_GGX(PrivatePBRData pbrData) {
    float d = (pbrData.NdH * pbrData.m2 - pbrData.NdH) * pbrData.NdH + 1.0;
    return pbrData.m2 / (PI * d * d);
}
float G_schlick(PrivatePBRData pbrData) {
    float x = (pbrData.m + 1.0);
    float k = (x * x) / 8.0;
    return (pbrData.NdV) / (pbrData.NdV * (1.0 - k) + k);
}

// cook-torrance specular calculation                      
float cooktorrance_specular(PrivatePBRData pbrData, FragmentPBRData fragData) {
    float D = D_GGX(pbrData);

    float G = G_schlick(pbrData);

    float rim = mix(1.0 - fragData.roughness * 0.9, 1.0, pbrData.NdV);

    return (1.0 / rim) * fragData.specular * G * D;
}

vec3 pbrShadeSpecular(PrivatePBRData pbrData, FragmentPBRData fragData, PrivateLightingData lightingData) {
	float specref = cooktorrance_specular(pbrData, fragData);
	
    specref *= pbrData.NdL;

	return specref * lightingData.radiance * lightingData.fresnel;
}
vec3 pbrShadeDiffuse(PrivatePBRData pbrData, FragmentPBRData fragData, PrivateLightingData lightingData) {
    return pbrLambertianDiffuse() * lightingData.radiance;
}

vec3 pbrCalculateLighting(FragmentPBRData fragment) {
	vec3 cameraPosWorld = ubo.invViewMatrix[3].xyz;
	vec3 N = normalize(fragment.normal);
	vec3 V = normalize(cameraPosWorld - fragment.position);
	vec3 I = normalize(fragment.position - cameraPosWorld);
	vec3 R = reflect(I, N);
	
	vec3 F0 = vec3(0.0); // no base reflection
	
	PrivatePBRData pbrDataBase;
	
	pbrDataBase.m = fragment.roughness * fragment.roughness; 
	pbrDataBase.m2 = pbrDataBase.m * pbrDataBase.m;
	
	pbrDataBase.V = V; 
	pbrDataBase.N = N; 
	
	vec3 diffuse = vec3(0.0);
	vec3 specular = vec3(0.0);
	
	//for (uint i = 0; i < scene.pointLightCount; i++) {
	//	vec3 L = scene.pointLights[i].position - fragment.position;
	//	float attenuation = 1.0 / dot(L, L);
	//	
	//	vec3 radiance = attenuation * scene.pointLights[i].color.rgb * scene.pointLights[i].color.a;
	//	
	//	vec3 H = normalize(V + L);
	//	
	//	PrivatePBRData pbrData = pbrDataBase;
	//	pbrData.NdL = max(dot(N, L), 0.0);
	//	pbrData.NdV = max(dot(N, V), 0.0);
	//	pbrData.NdH = max(dot(N, H), 0.0);
	//	pbrData.VdH = max(dot(V, H), 0.0);
	//	
	//	PrivateLightingData lightingData;
	//	lightingData.radiance = radiance;
	//	lightingData.fresnel = F_Schlick(F0, pbrData);
	//	
	//	diffuse += pbrShadeDiffuse(pbrData, fragment, lightingData);
	//	specular += pbrShadeSpecular(pbrData, fragment, lightingData);
	//}
	
	for (uint i = 0; i < scene.directionalLightCount; i++) {
		float intensity = max(dot(fragment.normal, scene.directionalLights[i].direction), 0.0);
		vec3 radiance = scene.directionalLights[i].color.rgb * scene.directionalLights[i].color.a;
		vec3 L = -scene.directionalLights[i].direction;
		vec3 H = normalize(V + L);
		
		PrivatePBRData pbrData = pbrDataBase;
		pbrData.NdL = max(dot(N, L), 0.0);
		pbrData.NdV = max(dot(N, V), 0.0);
		pbrData.NdH = max(dot(N, H), 0.0);
		pbrData.VdH = max(dot(V, H), 0.0);
		
		PrivateLightingData lightingData;
		lightingData.radiance = radiance;
		lightingData.fresnel = F_Schlick(F0, pbrData);
		
		diffuse += pbrShadeDiffuse(pbrData, fragment, lightingData)* intensity;
		specular += pbrShadeSpecular(pbrData, fragment, lightingData);
	}
	vec3 reflectedSpecular = mix(specular, specular * fragment.diffuse, fragment.metallic) * fragment.specular;
	vec3 reflectedDiffuse = mix(diffuse * fragment.diffuse, vec3(0.0), fragment.metallic);
	return reflectedDiffuse + reflectedSpecular;
}
#endif