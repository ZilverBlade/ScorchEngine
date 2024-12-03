#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) out vec3 ray;

#include "global_ubo.glsl"
#include "cube.glsl"

layout (push_constant) uniform Push {
	mat4 inverseTransform;
	vec3 halfExtent;
} push;

void main() {
	vec4 worldPos = inverse(push.inverseTransform) * vec4(vertices[gl_VertexIndex] * push.halfExtent, 1.0);
	ray = worldPos.xyz - ubo.invViewMatrix[3].xyz;
	gl_Position =ubo.viewProjMatrix * worldPos;
}