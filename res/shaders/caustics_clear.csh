#version 450
#extension GL_GOOGLE_include_directive : enable

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
layout (set = 0, binding = 0) uniform sampler2D refraction;
layout (set = 0, binding = 1) uniform sampler2D fresnel;
layout (set = 0, binding = 2, r32ui) uniform uimage2D causticMap;


void main() {
	imageStore(causticMap, ivec2(gl_GlobalInvocationID.xy), ivec4(0));
}