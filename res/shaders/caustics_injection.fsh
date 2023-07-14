#version 450
#extension GL_GOOGLE_include_directive : enable
#include "fixed_precision.glsl"

layout (set = 0, binding = 0) uniform sampler2D refraction;
layout (set = 0, binding = 1) uniform sampler2D fresnel;
layout (set = 0, binding = 2, r32ui) uniform uimage2D causticMap;
layout (location = 0) in vec2 fragUV;

layout (location = 0) out uint x;

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

void main() {	
	vec2 ts = vec2(imageSize(causticMap).xy);
	for (int i = 0; i < 8; i++) {
		vec2 rv = textureLod(refraction, fragUV + 8.0 * poissonDisk[i] / ts, 0.0).xy;
		vec2 clip = fragUV * 2.0 - 1.0;
		clip += rv;
		if (!any(greaterThan(abs(clip), vec2(1.0)))) {		
			float i = exp(textureLod(fresnel, fragUV, 0.0).x) - 1.0;
			
			int c = floatToFixed(i);
			
			vec2 uv = clip * 0.5 + 0.5;
			
			imageAtomicAdd(causticMap, ivec2(uv * vec2(ts - vec2(1.0))), c);
		}
	}
}