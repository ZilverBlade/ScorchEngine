#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) out vec3 ray;
layout (location = 1) out vec3 local;
layout (location = 2) out vec3 world;

#include "global_ubo.glsl"
#include "cube.glsl"

layout (push_constant) uniform Push {
	vec4 translation;
	vec3 halfExtent;
} push;

void main() {
	vec4 worldPos = vec4(vertices[gl_VertexIndex] * push.halfExtent + push.translation.xyz, 1.0);
	ray = worldPos.xyz - ubo.invViewMatrix[3].xyz;
	local = vertices[gl_VertexIndex];
	world = worldPos.xyz;
	gl_Position = ubo.viewProjMatrix * worldPos;
	
}