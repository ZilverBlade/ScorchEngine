#version 450

#include "global_ubo.glsl"

layout (location = 0) in vec3 pos;
layout (location = 0) out vec3 uv;

const vec3 vertices[36] = {
      {-.5f, -.5f, -.5f},
      {-.5f, .5f, .5f},
      {-.5f, -.5f, .5f},
      {-.5f, -.5f, -.5f},
      {-.5f, .5f, -.5f},
      {-.5f, .5f, .5f},
	  
      {.5f, -.5f, -.5f},
      {.5f, .5f, .5f},
      {.5f, -.5f, .5f},
      {.5f, -.5f, -.5f},
      {.5f, .5f, -.5f},
      {.5f, .5f, .5f},
	  
      {-.5f, -.5f, -.5f},
      {.5f, -.5f, .5f},
      {-.5f, -.5f, .5f},
      {-.5f, -.5f, -.5f},
      {.5f, -.5f, -.5f},
      {.5f, -.5f, .5f},
	  
      {-.5f, .5f, -.5f},
      {.5f, .5f, .5f},
      {-.5f, .5f, .5f},
      {-.5f, .5f, -.5f},
      {.5f, .5f, -.5f},
      {.5f, .5f, .5f},
	  
      {-.5f, -.5f, 0.5f},
      {.5f, .5f, 0.5f},
      {-.5f, .5f, 0.5f},
      {-.5f, -.5f, 0.5f},
      {.5f, -.5f, 0.5f},
      {.5f, .5f, 0.5f},
	  
      {-.5f, -.5f, -0.5f},
      {.5f, .5f, -0.5f},
      {-.5f, .5f, -0.5f},
      {-.5f, -.5f, -0.5f},
      {.5f, -.5f, -0.5f},
      {.5f, .5f, -0.5f}
};

void main() {
    uv = vertices[gl_VertexIndex];
    vec4 position = ubo.projMatrix * mat4(mat3(ubo.viewMatrix)) * vec4(vertices[gl_VertexIndex], 1.0);
	gl_Position = position.xyww;
}  