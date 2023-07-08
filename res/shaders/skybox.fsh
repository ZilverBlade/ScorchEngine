#version 450
#extension GL_GOOGLE_include_directive : enable

#include "global_ubo.glsl"
#include "scene_ssbo.glsl"

layout (location = 0) out vec4 outColor;
layout (location = 0) in vec3 uv;

layout(set = 2, binding = 0) uniform samplerCube skybox;


void main() {    
    outColor = vec4(textureLod(skybox, uv, 0.0).rgb * scene.skyLight.tint.rgb * scene.skyLight.tint.a, 1.0);
}