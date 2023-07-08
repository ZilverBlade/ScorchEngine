#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location  = 0) out int xxx;
layout (set = 0, binding = 0) uniform LPVInjectData {
	mat4 rsmInvProj;
	mat4 rsmInvView;
	mat4 rsmVP;
	vec3 lpvExtent;
	vec3 lpvCenter;
	vec3 lpvCellSize;
	vec3 lightDirection;
	vec3 lightIntensity;
	float temporalMix;
} injection;
layout (set = 0, binding = 1) uniform sampler2D rsmDepth;
layout (set = 0, binding = 2) uniform sampler2D rsmNormal;
layout (set = 0, binding = 3) uniform sampler2D rsmFlux;
layout (set = 0, binding = 4, rgba16f) uniform image3D LPV_RedSH;
layout (set = 0, binding = 5, rgba16f) uniform image3D LPV_GreenSH;
layout (set = 0, binding = 6, rgba16f) uniform image3D LPV_BlueSH;
layout (set = 0, binding = 7, rgba16f) uniform image3D LPV_PropagatedRedSH;
layout (set = 0, binding = 8, rgba16f) uniform image3D LPV_PropagatedGreenSH;
layout (set = 0, binding = 9, rgba16f) uniform image3D LPV_PropagatedBlueSH;

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
	for (int i = 0; i < imageSize(LPV_RedSH).z; i++) {
		cellIndex.z = i;
		// contribution
		vec4 cR = vec4(0.0);
		vec4 cG = vec4(0.0);
		vec4 cB = vec4(0.0);
		
		for (uint neighbour = 0; neighbour < 6; neighbour++) {
			mat3 orientation = neighbourOrientations[neighbour];
			// TODO: transpose all orientation matrices and use row indexing instead? ie int3( orientation[2] )
			vec3 mainDirection = orientation * vec3(0, 0, 1);
			
			ivec3 neighbourIndex = cellIndex - ivec3(directions[neighbour]);
			vec4 rCoeffsNeighbour = imageLoad(LPV_RedSH, neighbourIndex);
			vec4 gCoeffsNeighbour = imageLoad(LPV_GreenSH, neighbourIndex);
			vec4 bCoeffsNeighbour = imageLoad(LPV_BlueSH, neighbourIndex);
			
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
		
		const float TEMPORAL_MIX = 0.125;
		vec4 oldCR = imageLoad(LPV_PropagatedRedSH, cellIndex.xyz);
		vec4 oldCG = imageLoad(LPV_PropagatedGreenSH, cellIndex.xyz);
		vec4 oldCB = imageLoad(LPV_PropagatedBlueSH, cellIndex.xyz);
		imageStore(LPV_PropagatedRedSH, cellIndex.xyz, mix(oldCR, cR, TEMPORAL_MIX));
		imageStore(LPV_PropagatedGreenSH, cellIndex.xyz, mix(oldCG, cG, TEMPORAL_MIX));
		imageStore(LPV_PropagatedBlueSH, cellIndex.xyz, mix(oldCB, cB, TEMPORAL_MIX));
	
	}
	xxx = 0;
}
