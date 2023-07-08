#version 450
#extension GL_GOOGLE_include_directive : enable

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout (set = 0, binding = 4, rgba16f) uniform image3D LPV_RedSH;
layout (set = 0, binding = 5, rgba16f) uniform image3D LPV_GreenSH;
layout (set = 0, binding = 6, rgba16f) uniform image3D LPV_BlueSH;
layout (set = 0, binding = 7, rgba16f) uniform image3D LPV_PropagatedRedSH;
layout (set = 0, binding = 8, rgba16f) uniform image3D LPV_PropagatedGreenSH;
layout (set = 0, binding = 9, rgba16f) uniform image3D LPV_PropagatedBlueSH;

void main() {
	ivec3 coord = ivec3(gl_GlobalInvocationID.xyz);
	imageStore(LPV_RedSH, coord, vec4(0));
	imageStore(LPV_GreenSH, coord, vec4(0));
	imageStore(LPV_BlueSH, coord, vec4(0));
	
	//vec4 oldCR = imageLoad(LPV_PropagatedRedSH, cellIndex.xyz);
	//vec4 oldCG = imageLoad(LPV_PropagatedGreenSH, cellIndex.xyz);
	//vec4 oldCB = imageLoad(LPV_PropagatedBlueSH, cellIndex.xyz);
	//imageStore(LPV_RedSH, coord.xyz, oldCR + cR);
	//imageStore(LPV_GreenSH, coord.xyz, oldCG + cG);
	//imageStore(LPV_BlueSH, coord.xyz, oldCB + cB);
}
