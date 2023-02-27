#version 450
#extension GL_GOOGLE_include_directive : enable
#include "pbr_math.glsl"

layout (location = 0) in vec2 fragUV;
layout (location = 0) out vec2 outMap;

float radicalInverseVdC(uint bits)  {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 hammersley(uint i, uint N) {
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
} 

vec3 importanceSampleGGX(vec2 Xi, vec3 N, float m) {	
    const float phi = 2.0 * PI * Xi.x;
    const float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (m*m - 1.0) * Xi.y));
    const float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    const vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
	
    // from tangent-space vector to world-space sample vector
    const vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    const vec3 tangent   = normalize(cross(up, N));
    const vec3 bitangent = cross(N, tangent);
	
    const vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}  

void main() {
	const float NdV = fragUV.x;
	const float r = 1.0 - fragUV.y; // flip vulkan viewport
	
	const float NdV2 = NdV * NdV;
	const float m = r * r;
	const vec3 V = vec3(sqrt(1.0 - NdV2), 0.0, NdV);
    const vec3 N = vec3(0.0, 0.0, 1.0);
	
	const int sampleCount = 1024;
	
	vec2 integratedBRDF  = vec2(0.0);
    for(int i = 0; i < SAMPLE_COUNT; i++) {
		const vec2 Xi = Hammersley(i, SAMPLE_COUNT);
		const vec3 H  = ImportanceSampleGGX(Xi, N, m);
		const vec3 L  = normalize(2.0 * dot(V, H) * H - V);
		
		float NdL = max(L.z, 0.0);
		float NdH = max(H.z, 0.0);
		float VdH = max(dot(V, H), 0.0);
		
		if (NdL > 0.0) {
			float Vis = Vis_Schlick(NdV, NdL, m);
			float Fc = pow(1.0 - VdH, 5.0);
			
			integratedBRDF.x += (1.0 - Fc) * G_Vis;
			integratedBRDF.y += Fc * G_Vis;
		}
    }
    integratedBRDF /= float(SAMPLE_COUNT);
	outMap = integratedBRDF;
}