#extension GL_GOOGLE_include_directive : enable

#ifndef FIXED_PRECISION_GLSL
#define FIXED_PRECISION_GLSL

#define template_fdecimalfuncs(genTypeF, genTypeI, decimal_range)\
	genTypeI floatToFixed(genTypeF v) {\
		genTypeF scaled = v * genTypeF(decimal_range);\
		return genTypeI(scaled);\
	}\
	genTypeF fixedToFloat(genTypeI v) {\
		genTypeF fp = genTypeF(v);\
		return fp / genTypeF(decimal_range);\
	}

template_fdecimalfuncs(float, int, 500.0)
template_fdecimalfuncs(vec2, ivec2, 500.0)
template_fdecimalfuncs(vec3, ivec3, 500.0)
template_fdecimalfuncs(vec4, ivec4, 500.0)

#endif