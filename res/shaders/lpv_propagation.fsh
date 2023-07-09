#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location  = 0) out int xxx;

layout (push_constant) uniform Push {
	vec3 lpvExtent;
	vec3 lpvCellSize;
	ivec3 virtualPropagatedGridRedCoords;
	ivec3 virtualPropagatedGridGreenCoords;
	ivec3 virtualPropagatedGridBlueCoords;
	int pingPongIndex;
} push;

layout (set = 0, binding = 4, rgba16f) uniform image3D LPV_Inout_RedSH;
layout (set = 0, binding = 5, rgba16f) uniform image3D LPV_Inout_GreenSH;
layout (set = 0, binding = 6, rgba16f) uniform image3D LPV_Inout_BlueSH;
layout (set = 0, binding = 7, rgba16f) uniform image3D LPV_Inout2_RedSH;
layout (set = 0, binding = 8, rgba16f) uniform image3D LPV_Inout2_GreenSH;
layout (set = 0, binding = 9, rgba16f) uniform image3D LPV_Inout2_BlueSH;

const float SH_C0 = 0.282094792; // 1 / 2sqrt(pi)
const float SH_C1 = 0.488602512; // sqrt(3/pi) / 2

/*Cosine lobe coeff*/
const float SH_cosLobe_C0 = 0.886226925; // sqrt(pi)/2
const float SH_cosLobe_C1 = 1.023326707; // sqrt(pi/3)
const float PI = 3.1415926;

const vec3 directions[] =
{ vec3(0,0,1), vec3(0,0,-1), vec3(1,0,0), vec3(-1,0,0) , vec3(0,1,0), vec3(0,-1,0)};

// With a lot of help from: http://blog.blackhc.net/2010/07/light-propagation-volumes/
// This is a fully functioning LPV implementation

// right up
vec2 side[4] = { vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(-1.0, 0.0), vec2(0.0, -1.0) };

// orientation = [ right | up | forward ] = [ x | y | z ]
vec3 getEvalSideDirection(uint idx, mat3 orientation) {
	const float smallComponent = 0.4472135; // 1 / sqrt(5)
	const float bigComponent = 0.894427; // 2 / sqrt(5)
	
	const vec2 s = side[idx];
	// *either* x = 0 or y = 0
	return orientation * vec3(s.x * smallComponent, s.y * smallComponent, bigComponent);
}

vec3 getReprojSideDirection(uint idx, mat3 orientation) {
	const vec2 s = side[idx];
	return orientation * vec3(s.x, s.y, 0.0);
}

// orientation = [ right | up | forward ] = [ x | y | z ]
mat3 neighbourOrientations[6] = {
// Z+
mat3(1, 0, 0, 0, 1, 0,0, 0, 1),
// Z-
mat3(-1, 0, 0,0, 1, 0,0, 0, -1),
// X+
mat3(0, 0, 1,0, 1, 0,-1, 0, 0),
// X-
mat3(0, 0, -1,0, 1, 0,1, 0, 0),
// Y+
mat3(1, 0, 0,0, 0, 1,0, -1, 0),
// Y-
mat3(1, 0, 0,0, 0, -1,0, 1, 0)
};


vec4 dirToCosineLobe(vec3 dir) {
	return vec4(SH_cosLobe_C0, -SH_cosLobe_C1 * dir.y, SH_cosLobe_C1 * dir.z, -SH_cosLobe_C1 * dir.x);
}

vec4 dirToSH(vec3 dir) {
	return vec4(SH_C0, -SH_C1 * dir.y, SH_C1 * dir.z, -SH_C1 * dir.x);
}


void main() {
	ivec3 cellIndex = ivec3(gl_FragCoord.xyz);
	
	for (int i = 0; i < imageSize(LPV_Inout_RedSH).z; i++) {
		cellIndex.z = i;
		
		vec4 old_cR = vec4(0.0);
		vec4 old_cG = vec4(0.0);
		vec4 old_cB = vec4(0.0);
		
		if (push.pingPongIndex == 1) {
			old_cR = imageLoad(LPV_Inout_RedSH, cellIndex).xyzw;
		} else {
			old_cR = imageLoad(LPV_Inout2_RedSH, cellIndex).xyzw;
		}
		if (push.pingPongIndex == 1) {
			old_cG = imageLoad(LPV_Inout_GreenSH, cellIndex).xyzw;
		} else {
			old_cG = imageLoad(LPV_Inout2_GreenSH, cellIndex).xyzw;
		}
		if (push.pingPongIndex == 1) {
			old_cB = imageLoad(LPV_Inout_BlueSH, cellIndex).xyzw;
		} else {
			old_cB = imageLoad(LPV_Inout2_BlueSH, cellIndex).xyzw;
		}
		
		// contribution
		vec4 cR = vec4(0.0);
		vec4 cG = vec4(0.0);
		vec4 cB = vec4(0.0);
		
		for (uint neighbour = 0; neighbour < 6; neighbour++) {
			mat3 orientation = neighbourOrientations[neighbour];
			vec3 mainDirection = orientation * vec3(0, 0, 1);
			
			ivec3 neighbourIndex = cellIndex - ivec3(directions[neighbour]);
			vec4 rCoeffsNeighbour;
			vec4 gCoeffsNeighbour;
			vec4 bCoeffsNeighbour;
			if (push.pingPongIndex == 0) {
				rCoeffsNeighbour = imageLoad(LPV_Inout_RedSH, neighbourIndex).xyzw;
			} else {
				rCoeffsNeighbour = imageLoad(LPV_Inout2_RedSH, neighbourIndex).xyzw;
			}
			if (push.pingPongIndex == 0) {
				gCoeffsNeighbour = imageLoad(LPV_Inout_GreenSH, neighbourIndex).xyzw;
			} else {
				gCoeffsNeighbour = imageLoad(LPV_Inout2_GreenSH, neighbourIndex).xyzw;
			}
			if (push.pingPongIndex == 0) {
				bCoeffsNeighbour = imageLoad(LPV_Inout_BlueSH, neighbourIndex).xyzw;
			} else {
				bCoeffsNeighbour = imageLoad(LPV_Inout2_BlueSH, neighbourIndex).xyzw;
			}
			
			const float directFaceSubtendedSolidAngle = 0.4006696846 / PI / 2.0;
			const float sideFaceSubtendedSolidAngle = 0.4234413544 / PI / 3.0;
			
			for (uint sideFace = 0; sideFace < 4; ++sideFace) {
				vec3 evalDirection = getEvalSideDirection(sideFace, orientation);
				vec3 reprojDirection = getReprojSideDirection(sideFace, orientation);
				
				vec4 reprojDirectionCosineLobeSH = dirToCosineLobe(reprojDirection);
				vec4 evalDirectionSH = dirToSH(evalDirection);
				
				cR += sideFaceSubtendedSolidAngle * dot(rCoeffsNeighbour, evalDirectionSH) * reprojDirectionCosineLobeSH;
				cG += sideFaceSubtendedSolidAngle * dot(gCoeffsNeighbour, evalDirectionSH) * reprojDirectionCosineLobeSH;
				cB += sideFaceSubtendedSolidAngle * dot(bCoeffsNeighbour, evalDirectionSH) * reprojDirectionCosineLobeSH;
			}
			
			vec3 curDir = directions[neighbour];
			vec4 curCosLobe = dirToCosineLobe(curDir);
			vec4 curDirSH = dirToSH(curDir);
			
			ivec3 neighbourCellIndex = cellIndex + ivec3(curDir);
			
			cR += directFaceSubtendedSolidAngle * max(0.0, dot(rCoeffsNeighbour, curDirSH)) * curCosLobe;
			cG += directFaceSubtendedSolidAngle * max(0.0, dot(gCoeffsNeighbour, curDirSH)) * curCosLobe;
			cB += directFaceSubtendedSolidAngle * max(0.0, dot(bCoeffsNeighbour, curDirSH)) * curCosLobe;
		}
		if (push.pingPongIndex == 1) {
			imageStore(LPV_Inout_RedSH, cellIndex, old_cR + cR);
		} else {
			imageStore(LPV_Inout2_RedSH, cellIndex, old_cR + cR);
		}
		if (push.pingPongIndex == 1) {
			imageStore(LPV_Inout_GreenSH, cellIndex, old_cG + cG);
		} else {
			imageStore(LPV_Inout2_GreenSH, cellIndex, old_cG + cG);
		}
		if (push.pingPongIndex == 1) {
			imageStore(LPV_Inout_BlueSH, cellIndex, old_cB + cB);
		} else {
			imageStore(LPV_Inout2_BlueSH, cellIndex, old_cB + cB);
		}
	
	}
	xxx = 0;
}
