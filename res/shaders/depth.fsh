#version 450
#extension GL_GOOGLE_include_directive : enable
#define SURFACE_MATERIAL_DESCRIPTOR_SET 1
#include "surface_material.glsl"

layout (location = 1) in vec2 fragUV;

void main() {
	if (sfHasMaskTexture()){
		if (texture(sfMaskTexture, sfSampleUV(fragUV)).x < 0.5) {
			discard;
		}
	}
}