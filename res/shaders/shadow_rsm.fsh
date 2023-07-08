#version 450
#extension GL_GOOGLE_include_directive : enable
#define SURFACE_MATERIAL_DESCRIPTOR_SET 0
#include "surface_material.glsl"

layout (location = 0) in vec2 fragUV;
layout (location = 1) in vec3 fragNormal;

layout (location = 0) out vec4 normal;
layout (location = 1) out vec4 flux;

void main() {
	vec2 uv = sfSampleUV(fragUV);
	if (sfHasMaskTexture()){
		if (texture(sfMaskTexture, uv).x < 0.5) {
			discard;
		}
	}
	vec3 diffuse = surfaceMaterialParams.diffuse.rgb;
	if (sfHasDiffuseTexture()) {
		diffuse *= texture(sfDiffuseTexture, uv).rgb;
	} 
	normal = vec4(normalize(fragNormal), 1.0);
	flux = vec4(diffuse, 1.0);
}