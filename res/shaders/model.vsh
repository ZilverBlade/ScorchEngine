#version 450
#extension GL_GOOGLE_include_directive : enable
#include "global_ubo.glsl"

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;

layout (location = 0) out vec3 fragPosWorld;
layout (location = 1) out vec2 fragUV;
layout (location = 2) out vec3 fragNormal;
layout (location = 3) out vec3 fragTangent;

layout (push_constant) uniform Push {
	mat4 modelMatrix;
	mat3 normalMatrix;
} push;

void main() {
	vec4 worldPosition = push.modelMatrix * vec4(position, 1.0);
	gl_Position = ubo.viewProjMatrix * worldPosition;
	
	fragPosWorld = worldPosition.xyz;
	fragUV = uv;
	
	mat3 normalMatrix = inverse(transpose(mat3(push.modelMatrix)) // figure out why push.normalMatrix is broken
	fragNormal = normalize(normalMatrix * normal); 
	fragTangent = normalize(normalMatrix * tangent);
}