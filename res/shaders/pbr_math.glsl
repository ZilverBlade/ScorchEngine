#extension GL_GOOGLE_include_directive : enable

#ifndef PBR_MATH_GLSL
#define PBR_MATH_GLSL
#include "constants.glsl"

float D_beckmann(float NdH, float m2) {
    float NdH2 = pbrData.NdH * pbrData.NdH;
    return exp((NdH2 - 1.0) / max(pbrSData.m2 * NdH2, 1e-6)) / max(PI * pbrSData.m2 * NdH2 * NdH2, 1e-6);
}

float D_GGX(float NdH, float m2) {
    float d = (pbrData.NdH * pbrSData.m2 - pbrData.NdH) * pbrData.NdH + 1.0;
    return pbrSData.m2 / max(PI * d * d, 1e-6);
}
float Vis_Schlick(float NdV, float NdL, float m) {
    float x = (m + 1.0);
    float k = (x * x) / 8.0;
	
	float Vis_SchlickV = NdV * (1.0 - k) + k;
	float Vis_SchlickL = NdL * (1.0 - k) + k;
	return 0.25 / max(Vis_SchlickV * Vis_SchlickL, 1e-6);
}

#endif