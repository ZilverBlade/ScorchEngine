#version 450
#extension GL_GOOGLE_include_directive : enable
#define SURFACE_MATERIAL_DESCRIPTOR_SET 0
#include "surface_material.glsl"

layout (location = 0) in vec2 fragUV;
layout (location = 1) in vec3 fragNormal;

layout (location = 0) out vec2 refraction;
layout (location = 1) out float fresnel;

layout (push_constant) uniform Push {
	mat4 vp;
	mat4 modelMatrix;
	// too bad amd!
	mat4 vpnotrans;
	vec4 direction; 
	float ior;
} push;

void main() {
	vec2 uv = sfSampleUV(fragUV);
	if (sfHasMaskTexture()){
		if (texture(sfMaskTexture, uv).x < 0.5) {
			discard;
		}
	}
	
	vec3 N = normalize(fragNormal);
	if (gl_FrontFacing == false) {
		N = -N;
	}
	
	float NdD = abs(dot(N, -push.direction.xyz));
	
    float f = 1.0 - NdD;
	float ff2 = f * f;
	float ff5 = ff2 * ff2 * f;
    float fres = f + ff5;
	
	float factor = 1.0 - fres;
 
	vec4 Rf = vec4(refract(-push.direction.xyz, N, 1.0 + factor * (push.ior - 1.0)), 1.0);
	vec4 scRf = push.vpnotrans * Rf;
	
	refraction = scRf.xy;
	fresnel = 1.0 - fres;
}