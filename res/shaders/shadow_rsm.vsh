#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;

layout (location = 0) out vec2 fragUV;
layout (location = 1) out vec3 fragNormal;

layout (push_constant) uniform Push {
	mat4 vp;
	mat4 modelMatrix;
} push;

void main() {
	gl_Position = push.vp * push.modelMatrix * vec4(position, 1.0);
	fragUV = uv;
	mat3 normalMatrix = inverse(transpose(mat3(push.modelMatrix))); // figure out why push.normalMatrix is broken
	fragNormal = normalize(normalMatrix * normal); 
}