#ifndef SURFACE_MATERIAL_GLSL
#define SURFACE_MATERIAL_GLSL

//#define SURFACE_MATERIAL_DESCRIPTOR_SET N

const uint SURFACE_MATERIAL_SHADING_MODEL_LIT = 0x01;
const uint SURFACE_MATERIAL_SHADING_MODEL_UNLIT = 0x02;
const uint SURFACE_MATERIAL_SHADING_MODEL_CLEARCOAT = 0x03;

const uint SURFACE_MATERIAL_TEXTURE_DIFFUSE_BIT = 0x01;
const uint SURFACE_MATERIAL_TEXTURE_EMISSIVE_BIT = 0x02;
const uint SURFACE_MATERIAL_TEXTURE_NORMAL_BIT = 0x04;
const uint SURFACE_MATERIAL_TEXTURE_SPECULAR_BIT = 0x08;
const uint SURFACE_MATERIAL_TEXTURE_ROUGHNESS_BIT = 0x10;
const uint SURFACE_MATERIAL_TEXTURE_METALLIC_BIT = 0x20;
const uint SURFACE_MATERIAL_TEXTURE_AMBIENTOCCLUSION_BIT = 0x40;
const uint SURFACE_MATERIAL_TEXTURE_MASK_BIT = 0x80;
const uint SURFACE_MATERIAL_TEXTURE_OPACITY_BIT = 0x100;

layout(set = SURFACE_MATERIAL_DESCRIPTOR_SET, binding = 0) uniform Params {
	vec3 diffuse;
	vec3 emission;
	float specular;
	float roughness;
	float metallic;
	float ambientOcclusion;
	vec2 uvScale;
	vec2 uvOffset;
	uint textureFlags;
	uint shadingModelFlag;
} surfaceMaterialParams;

layout(set = SURFACE_MATERIAL_DESCRIPTOR_SET, binding = 1) uniform sampler2D sfDiffuseTexture;
layout(set = SURFACE_MATERIAL_DESCRIPTOR_SET, binding = 2) uniform sampler2D sfEmissiveTexture;
layout(set = SURFACE_MATERIAL_DESCRIPTOR_SET, binding = 3) uniform sampler2D sfNormalTexture;
layout(set = SURFACE_MATERIAL_DESCRIPTOR_SET, binding = 4) uniform sampler2D sfSpecularTexture;
layout(set = SURFACE_MATERIAL_DESCRIPTOR_SET, binding = 5) uniform sampler2D sfRoughnessTexture;
layout(set = SURFACE_MATERIAL_DESCRIPTOR_SET, binding = 6) uniform sampler2D sfMetallicTexture;
layout(set = SURFACE_MATERIAL_DESCRIPTOR_SET, binding = 7) uniform sampler2D sfAmbientOcclusionTexture;
layout(set = SURFACE_MATERIAL_DESCRIPTOR_SET, binding = 8) uniform sampler2D sfMaskTexture;
//layout(set = SURFACE_MATERIAL_DESCRIPTOR_SET, binding = 9) uniform sampler2D sfOpacityTexture;

const bool sfHasDiffuseTexture() {
	return surfaceMaterialParams.textureFlags & SURFACE_MATERIAL_TEXTURE_DIFFUSE_BIT;
}
const bool sfHasEmissiveTexture() {
	return surfaceMaterialParams.textureFlags & SURFACE_MATERIAL_TEXTURE_EMISSIVE_BIT
}
const bool sfHasNormalTexture() {
	return surfaceMaterialParams.textureFlags & SURFACE_MATERIAL_TEXTURE_NORMAL_BIT;
}
const bool sfHasSpecularTexture() {
	return surfaceMaterialParams.textureFlags & SURFACE_MATERIAL_TEXTURE_SPECULAR_BIT;
}
const bool sfHasRoughnessTexture() {
	return surfaceMaterialParams.textureFlags & SURFACE_MATERIAL_TEXTURE_ROUGHNESS_BIT
}
const bool sfHasMetallicTexture() {
	return surfaceMaterialParams.textureFlags & SURFACE_MATERIAL_TEXTURE_METALLIC_BIT;
}
const bool sfHasAmbientOcclusionTexture() {
	return surfaceMaterialParams.textureFlags & SURFACE_MATERIAL_TEXTURE_AMBIENTOCCLUSION_BIT;
}
const bool sfHasMaskTexture() {
	return surfaceMaterialParams.textureFlags & SURFACE_MATERIAL_TEXTURE_MASK_BIT;
}

const bool sfShadingModelLit() {
	return surfaceMaterialParams.shadingModelFlag & SURFACE_MATERIAL_SHADING_MODEL_LIT;
}
const bool sfShadingModelUnlit() {
	return surfaceMaterialParams.shadingModelFlag & SURFACE_MATERIAL_SHADING_MODEL_UNLIT;
}
const bool sfShadingModelClearCoat() {
	return surfaceMaterialParams.shadingModelFlag & SURFACE_MATERIAL_SHADING_MODEL_CLEARCOAT;
}

vec2 sfSampleUV(vec2 uv) {
	return uv * surfaceMaterialParams.uvScale + surfaceMaterialParams.uvOffset;
}

#endif