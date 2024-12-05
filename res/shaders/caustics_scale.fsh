#version 450
#extension GL_GOOGLE_include_directive : enable
#include "fixed_precision.glsl"

layout (set = 0, binding = 0) uniform usampler2D causticMap;
layout (location = 0) in vec2 fragUV;

layout (location = 0) out float caustics;

void main() {	
	caustics = fixedToFloat(int(texture(causticMap, fragUV).x));
}