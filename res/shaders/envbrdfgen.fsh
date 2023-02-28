#version 450
#extension GL_GOOGLE_include_directive : enable
#include "pbr_math.glsl"
#include "magic.glsl"

layout (location = 0) in vec2 fragUV;
layout (location = 0) out vec2 outMap;


void main() {
	const float NdV = fragUV.x;
	const float r = 1.0 - fragUV.y; // flip vulkan viewport
	
	const float NdV2 = NdV * NdV;
	const float m = r * r;
	const vec3 V = vec3(sqrt(1.0 - NdV2), 0.0, NdV);
    const vec3 N = vec3(0.0, 0.0, 1.0);
	
	const int sampleCount = 1024;
	
	vec2 integratedBRDF  = vec2(0.0);
    for(int i = 0; i < sampleCount; i++) {
		const vec2 Xi = hammersley(i, sampleCount);
		const vec3 H  = importanceSampleGGX(Xi, N, m);
		const vec3 L  = normalize(2.0 * dot(V, H) * H - V);
		
		float NdL = max(L.z, 0.0);
		float NdH = max(H.z, 0.0);
		float VdH = max(dot(V, H), 0.0);
		
		if (NdL > 0.0) {
			float Vis = Vis_Schlick(NdV, NdL, m);
			float Fc = pow(1.0 - VdH, 5.0);
			
			integratedBRDF.x += (1.0 - Fc) * Vis;
			integratedBRDF.y += Fc * Vis;
		}
    }
    integratedBRDF /= float(sampleCount);
	outMap = integratedBRDF;
}