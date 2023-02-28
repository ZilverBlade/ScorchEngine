#version 450

#include "global_ubo.glsl"
#include "cube.glsl"

layout (location = 0) in vec3 pos;
layout (location = 0) out vec3 uv;

void main() {
    uv = vertices[gl_VertexIndex];
    vec4 position = ubo.projMatrix * mat4(mat3(ubo.viewMatrix)) * vec4(vertices[gl_VertexIndex], 1.0);
	gl_Position = position.xyww;
}  