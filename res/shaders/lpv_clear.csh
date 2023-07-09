#version 450
#extension GL_GOOGLE_include_directive : enable

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout (set = 0, binding = 4, rgba16f) uniform image3D LPV_Inout_RedSH;
layout (set = 0, binding = 5, rgba16f) uniform image3D LPV_Inout_GreenSH;
layout (set = 0, binding = 6, rgba16f) uniform image3D LPV_Inout_BlueSH;
layout (set = 0, binding = 7, rgba16f) uniform image3D LPV_Inout2_RedSH;
layout (set = 0, binding = 8, rgba16f) uniform image3D LPV_Inout2_GreenSH;
layout (set = 0, binding = 9, rgba16f) uniform image3D LPV_Inout2_BlueSH;

void main() {
	ivec3 coord = ivec3(gl_GlobalInvocationID.xyz);
	imageStore(LPV_Inout_RedSH, coord, vec4(0));
	imageStore(LPV_Inout_GreenSH, coord, vec4(0));
	imageStore(LPV_Inout_BlueSH, coord, vec4(0));
	imageStore(LPV_Inout2_RedSH, coord, vec4(0));
	imageStore(LPV_Inout2_GreenSH, coord, vec4(0));
	imageStore(LPV_Inout2_BlueSH, coord, vec4(0));
}
