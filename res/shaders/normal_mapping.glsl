#ifndef NORMAL_MAPPING_GLSL
#define NORMAL_MAPPING_GLSL

vec3 sampleNormalMap(vec3 normalMapData, mat3 TBN) {
	vec3 tgNormal = normalMapData * 2.0 - 1.0;
	return normalize(TBN * tgNormal);
}

#endif