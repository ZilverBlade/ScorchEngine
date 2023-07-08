#version 450
#extension GL_GOOGLE_include_directive : enable
#define SURFACE_MATERIAL_DESCRIPTOR_SET 4
#include "surface_material.glsl"
#include "normal_mapping.glsl"
#include "pbr.glsl"

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 fragPosWorld;
layout (location = 1) in vec2 fragUV;
layout (location = 2) in vec3 fragNormal;
layout (location = 3) in vec3 fragTangent;

void main() {
	vec2 uv = sfSampleUV(fragUV);
	if (sfHasMaskTexture()){
		if (texture(sfMaskTexture, uv).x < 0.5) {
			discard;
		}
	}

	vec3 emission = surfaceMaterialParams.emission.rgb;
	if (sfHasEmissiveTexture()){
		emission = texture(sfMaskTexture, uv).rgb;
	}

	
	if (sfShadingModelLit()) {	
		vec3 N = normalize(fragNormal);
		FragmentLitPBRData fragment;
		fragment.position = fragPosWorld;
		if (sfHasNormalTexture()) {
			vec3 T = normalize(fragTangent);
			mat3 TBN = mat3(T, cross(N, T), N);
			sampleNormalMap(texture(sfNormalTexture, uv).xyz, TBN);
		} else {
			fragment.normal = N;
		}
		fragment.diffuse = surfaceMaterialParams.diffuse.rgb;
		if (sfHasDiffuseTexture()) {
			fragment.diffuse *= texture(sfDiffuseTexture, uv).rgb;
		} 
		fragment.specular = surfaceMaterialParams.specular;
		if (sfHasSpecularTexture()) {
			fragment.specular *= texture(sfSpecularTexture, uv).x;
		} 
		fragment.roughness = surfaceMaterialParams.roughness;
		if (sfHasRoughnessTexture()) {
			fragment.roughness *= texture(sfRoughnessTexture, uv).x;
		} 
		fragment.metallic = surfaceMaterialParams.metallic;
		if (sfHasMetallicTexture()) {
			fragment.metallic *= texture(sfMetallicTexture, uv).x;
		}
		fragment.ambientOcclusion = surfaceMaterialParams.ambientOcclusion;
		if (sfHasAmbientOcclusionTexture()) {
			fragment.ambientOcclusion *= texture(sfAmbientOcclusionTexture, uv).x;
		}
		
		FragmentClearCoatPBRData fragmentcc;
		fragmentcc.clearCoat = 0.0;
		vec3 lighting = pbrCalculateLighting(fragment, fragmentcc);
		outColor = vec4(lighting + emission, 1.0);
	} 
	else if (sfShadingModelUnlit()) {
		outColor = vec4(emission, 1.0);
	} 
	else if (sfShadingModelClearCoat()) {	
		vec3 N = normalize(fragNormal);
		FragmentLitPBRData fragment;
		FragmentClearCoatPBRData fragmentcc;
		fragment.position = fragPosWorld;
		if (sfHasNormalTexture()) {
			vec3 T = normalize(fragTangent);
			mat3 TBN = mat3(T, cross(N, T), N);
			sampleNormalMap(texture(sfNormalTexture, uv).xyz, TBN);
		} else {
			fragment.normal = N;
		}
		fragment.diffuse = surfaceMaterialParams.diffuse.rgb;
		if (sfHasDiffuseTexture()) {
			fragment.diffuse *= texture(sfDiffuseTexture, uv).rgb;
		} 
		fragment.specular = surfaceMaterialParams.specular;
		if (sfHasSpecularTexture()) {
			fragment.specular *= texture(sfSpecularTexture, uv).x;
		} 
		fragment.roughness = surfaceMaterialParams.roughness;
		if (sfHasRoughnessTexture()) {
			fragment.roughness *= texture(sfRoughnessTexture, uv).x;
		} 
		fragment.metallic = surfaceMaterialParams.metallic;
		if (sfHasMetallicTexture()) {
			fragment.metallic *= texture(sfMetallicTexture, uv).x;
		}
		fragment.ambientOcclusion = surfaceMaterialParams.ambientOcclusion;
		if (sfHasAmbientOcclusionTexture()) {
			fragment.ambientOcclusion *= texture(sfAmbientOcclusionTexture, uv).x;
		}
		fragmentcc.clearCoat = surfaceMaterialParams.clearCoat;
		fragmentcc.clearCoatRoughness = surfaceMaterialParams.clearCoatRoughness;
		vec3 lighting = pbrCalculateLighting(fragment, fragmentcc);
		outColor = vec4(lighting + emission, 1.0);
	} 
}