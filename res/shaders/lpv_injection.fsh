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
layout (set = 0, binding = 0) uniform LPVInjectData {
	mat4 rsmInvProj;
	mat4 rsmInvView;
	mat4 rsmVP;
	vec3 lpvCenter;
	vec3 lightDirection;
	vec3 lightIntensity;
} injection;
layout (set = 0, binding = 1) uniform sampler2D rsmDepth;
layout (set = 0, binding = 2) uniform sampler2D rsmNormal;
layout (set = 0, binding = 3) uniform sampler2D rsmFlux;
layout (set = 0, binding = 4, rgba16f) uniform image3D LPV_Inout_RedSH;
layout (set = 0, binding = 5, rgba16f) uniform image3D LPV_Inout_GreenSH;
layout (set = 0, binding = 6, rgba16f) uniform image3D LPV_Inout_BlueSH;

const float SH_C0 = 0.282094792; // 1 / 2sqrt(pi)
const float SH_C1 = 0.488602512; // sqrt(3/pi) / 2

/*Cosine lobe coeff*/
const float SH_cosLobe_C0 = 0.886226925; // sqrt(pi)/2
const float SH_cosLobe_C1 = 1.023326707; // sqrt(pi/3)
const float PI = 3.1415926;

const float POS_BIAS_NORMAL = 1.0;
const float POS_BIAS_LIGHT = 1.0;

const float SURFEL_WEIGHT = 0.15;

const int KERNEL_SIZE = 4;
const int STEP_SIZE = 1;

struct RSMData {
	vec3 normal;
	vec3 position;
	vec3 flux;
};

float Luminance(RSMData data) {
	return data.flux.r * 0.299 + data.flux.g * 0.587 + data.flux.b * 0.114;
}

RSMData fetchRSMData(ivec2 coords) {
	vec2 uv = vec2(coords) / vec2(textureSize(rsmFlux, 0).xy - 1);
	RSMData data;
	data.flux = vec3(0.0);
	data.normal = textureLod(rsmNormal, uv, 0.0).xyz;
	vec4 viewSpace = injection.rsmInvProj * vec4(uv * 2.0 - 1.0, textureLod(rsmDepth, uv, 0.0).r, 1.0);
	vec4 world = injection.rsmInvView * viewSpace;
	data.position = world.xyz + (data.normal * POS_BIAS_NORMAL);
	float l = length(data.normal);
	if (l > 0.0) {
		data.normal /= l;
	} else {
		return data;
	}
	data.flux = textureLod(rsmFlux, uv, 0.0).rgb * injection.lightIntensity * max(0.0, dot(data.normal, -injection.lightDirection));
	
	
	return data;
}

vec4 dirToCosineLobe(vec3 dir) {
	return vec4(SH_cosLobe_C0, -SH_cosLobe_C1 * dir.y, SH_cosLobe_C1 * dir.z, -SH_cosLobe_C1 * dir.x);
}

vec4 dirToSH(vec3 dir) {
	return vec4(SH_C0, -SH_C1 * dir.y, SH_C1 * dir.z, -SH_C1 * dir.x);
}

vec3 uvToWorld(vec3 uv) {
	return (uv * 2.0 - 1.0) * push.lpvExtent + injection.lpvCenter;
}
vec3 worldToUV(vec3 world) {
	return ( (world - injection.lpvCenter) / push.lpvExtent + 1.0 ) / 2.0;
}

void main() {
	ivec3 rsmCoords = ivec3(gl_FragCoord.xyz);
	
	ivec2 RSMsize = textureSize(rsmDepth, 0).xy;
	RSMsize /= KERNEL_SIZE;
	
	// Pick brightest cell in KERNEL_SIZExKERNEL_SIZE grid
	vec3 brightestCellIndex;
	float maxLuminance = -0.01;
	
	for (uint y = 0; y < KERNEL_SIZE; y += STEP_SIZE) {
		for (uint x = 0; x < KERNEL_SIZE; x += STEP_SIZE) {
			ivec2 txi = rsmCoords.xy + ivec2(x, y);
			RSMData data = fetchRSMData(txi);
			if (data.normal == vec3(0.0)) continue;
			float texLum = Luminance(data);
			if (texLum > maxLuminance) {
				brightestCellIndex = worldToUV(data.position) * vec3(imageSize(LPV_Inout_RedSH).xyz - 1);
				maxLuminance = texLum;
			}
		}
	}
	
	RSMData result;
	result.position = vec3(0.0);
	result.normal = vec3(0.0);
	result.flux = vec3(0.0);
	float numSamples = 0;
	for (uint y = 0; y < KERNEL_SIZE; y += STEP_SIZE) {
		for (uint x = 0; x < KERNEL_SIZE; x += STEP_SIZE) {
			ivec2 txi = rsmCoords.xy + ivec2(x, y);
			RSMData data = fetchRSMData(txi);
			vec3 texelIndex = worldToUV(data.position) * vec3(imageSize(LPV_Inout_RedSH).xyz - 1);
			vec3 deltaGrid = texelIndex - brightestCellIndex;
			if (dot(deltaGrid, deltaGrid) < 10) { // If cell proximity is good enough 
				// Sample from texel
				result.flux += data.flux;
				result.position += data.position;
				result.normal += data.normal;
				numSamples+=1.0;
			}
		}
	}
	
	if (numSamples > 0)  {
		result.position /= numSamples;
		result.normal /= numSamples;
		float l = length(result.normal);
		if (l > 0.0) {
			result.normal /= l;
		}
		result.flux /= numSamples;
	}
	
	ivec3 worldCoords = ivec3(worldToUV(result.position) * imageSize(LPV_Inout_RedSH).xyz);
	vec4 coef = (dirToCosineLobe(result.normal) / PI) * SURFEL_WEIGHT;
	imageStore(LPV_Inout_RedSH, worldCoords, coef * result.flux.r);
	imageStore(LPV_Inout_GreenSH, worldCoords,  coef * result.flux.g);
	imageStore(LPV_Inout_BlueSH, worldCoords, coef * result.flux.b);
	
	xxx = 0;
}