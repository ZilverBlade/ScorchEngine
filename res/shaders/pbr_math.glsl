#extension GL_GOOGLE_include_directive : enable

#ifndef PBR_MATH_GLSL
#define PBR_MATH_GLSL
#include "constants.glsl"

float D_beckmann(float NdH, float m2) {
    float NdH2 = NdH * NdH;
    return exp((NdH2 - 1.0) / max(m2 * NdH2, 1e-6)) / max(PI * m2 * NdH2 * NdH2, 1e-6);
}

float D_GGX(float NdH, float m2) {
    float d = (NdH * m2 - NdH) * NdH + 1.0;
    return m2 / max(PI * d * d, 1e-6);
}
float Vis_Schlick(float NdV, float NdL, float m) {
    float x = (m + 1.0);
    float k = (x * x) / 8.0;
	
	float Vis_SchlickV = NdV * (1.0 - k) + k;
	float Vis_SchlickL = NdL * (1.0 - k) + k;
	return (max(Vis_SchlickV * Vis_SchlickL, 1e-6));
}
float G_SchlickGGX_IBL(float NdV, float NdL, float m) {
	float k = (m * m) / 2.0;
	float GL = NdL / max(NdL * (1.0 - k) + k, 1e-6);
	float GV = NdV / max(NdV * (1.0 - k) + k, 1e-6);
	return GL * GV;
}
float GVis_SchlickGGX(float NdV, float NdL, float NdH, float VdH, float m) {
    float x = (m + 1.0);
    float k = (x * x) / 8.0;
	float GL = NdL / max(NdL * (1.0 - k) + k, 1e-6);
	float GV = NdV / max(NdV * (1.0 - k) + k, 1e-6);
	return (GL * GV * VdH) / max(NdH * NdV, 1e-6);
}
#endif