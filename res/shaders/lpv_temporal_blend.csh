#version 450
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_image_load_formatted : enable
layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout (set = 0, binding = 4, rgba16f) uniform image3D LPV_Inout_RedSH;
layout (set = 0, binding = 5, rgba16f) uniform image3D LPV_Inout_GreenSH;
layout (set = 0, binding = 6, rgba16f) uniform image3D LPV_Inout_BlueSH;
layout (set = 0, binding = 7, rgba16f) uniform image3D LPV_Inout2_RedSH;
layout (set = 0, binding = 8, rgba16f) uniform image3D LPV_Inout2_GreenSH;
layout (set = 0, binding = 9, rgba16f) uniform image3D LPV_Inout2_BlueSH;
layout (set = 0, binding = 10, rgba16f) uniform image3D virtual_OutPropagatedLPV;

layout (push_constant) uniform Push {
	ivec3 virtualPropagatedGridRedCoords;
	ivec3 virtualPropagatedGridGreenCoords;
	ivec3 virtualPropagatedGridBlueCoords;
	int finalPingPongOutput;
	float temporalBlend;
} push;

void main() {	
	ivec3 cellIndex = ivec3(gl_GlobalInvocationID.xyz);
	vec4 rCoeffs;
	vec4 gCoeffs;
	vec4 bCoeffs;
	if (push.finalPingPongOutput == 0) {
		rCoeffs = imageLoad(LPV_Inout_RedSH, cellIndex).xyzw;
	} else {
		rCoeffs = imageLoad(LPV_Inout2_RedSH, cellIndex).xyzw;
	}
	if (push.finalPingPongOutput == 0) {
		gCoeffs = imageLoad(LPV_Inout_GreenSH, cellIndex).xyzw;
	} else {
		gCoeffs = imageLoad(LPV_Inout2_GreenSH, cellIndex).xyzw;
	}
	if (push.finalPingPongOutput == 0) {
		bCoeffs = imageLoad(LPV_Inout_BlueSH, cellIndex).xyzw;
	} else {
		bCoeffs = imageLoad(LPV_Inout2_BlueSH, cellIndex).xyzw;
	}
	vec4 old_rCoeffs = imageLoad(virtual_OutPropagatedLPV, push.virtualPropagatedGridRedCoords + cellIndex.xyz);
	vec4 old_gCoeffs = imageLoad(virtual_OutPropagatedLPV, push.virtualPropagatedGridGreenCoords + cellIndex.xyz);
	vec4 old_bCoeffs = imageLoad(virtual_OutPropagatedLPV, push.virtualPropagatedGridBlueCoords + cellIndex.xyz);
	
	imageStore(virtual_OutPropagatedLPV, push.virtualPropagatedGridRedCoords + cellIndex.xyz, mix(old_rCoeffs, rCoeffs, push.temporalBlend));
	imageStore(virtual_OutPropagatedLPV, push.virtualPropagatedGridGreenCoords + cellIndex.xyz, mix(old_gCoeffs, gCoeffs, push.temporalBlend));
	imageStore(virtual_OutPropagatedLPV, push.virtualPropagatedGridBlueCoords + cellIndex.xyz, mix(old_bCoeffs, bCoeffs, push.temporalBlend));
}
