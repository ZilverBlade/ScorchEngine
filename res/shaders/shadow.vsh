#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

layout (location = 0) out vec2 fragUV;

layout (push_constant) uniform Push {
	mat4 mvp;
} push;

void main() {
	gl_Position = push.mvp * vec4(position, 1.0);
	fragUV = uv;
}