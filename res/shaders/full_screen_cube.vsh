#version 450

#include "cube.glsl"

layout (location = 0) in vec3 pos;
layout (location = 0) out vec3 uv;

layout (push_constant) uniform Push{
	mat4 mvp;
} push;

void main() {
    uv = vertices[gl_VertexIndex] * 2.0;
    vec4 position = push.mvp * vec4(vertices[gl_VertexIndex] * 2.0, 1.0);
	
	position.z = clamp(position.z, 0, 1);
	gl_Position = position;
} 