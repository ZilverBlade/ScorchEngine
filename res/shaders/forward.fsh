#version 450
#extension GL_GOOGLE_include_directive : enable
#include "global_ubo.glsl"

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 fragPosWorld;
layout (location = 1) in vec2 fragUV;
layout (location = 2) in vec3 fragNormal;
layout (location = 3) in vec3 fragTangent;

void main() {
	outColor = vec4(fragUV, 0.0, 1.0);
}