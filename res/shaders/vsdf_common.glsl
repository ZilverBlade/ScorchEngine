#ifndef VSDF_COMMON_GLSL
#define VSDF_COMMON_GLSL

layout (set = 1, binding = 0) uniform sampler3D sdf;

#ifndef SDF_THRESHOLD
#define SDF_THRESHOLD 0.015
#endif

#ifndef SDF_MAX_STEPS
#define SDF_MAX_STEPS 64
#endif

struct SdfResult {
	float closest;
	float dist;
};

vec3 worldToVoxelSpace(vec3 p, mat4 inverseTransform, vec3 sdfHalfExtent) {
	vec3 cc = (inverseTransform * vec4(p, 1.0)).xyz / sdfHalfExtent;
	return cc * 0.5 + 0.5;
}

SdfResult sceneSdf(vec3 rd, vec3 ro, mat4 inverseTransform, vec3 sdfHalfExtent, float closestScale) {
	vec3 start = ro;
	SdfResult res;
	//float closestScale = dot(push.scale, abs(rotationMatrix * rd));
	for (int i = 0; i < SDF_MAX_STEPS; ++i) {
		vec3 uv = worldToVoxelSpace(ro, inverseTransform, sdfHalfExtent);
		res.closest = texture(sdf, uv).r;
		if (res.closest < SDF_THRESHOLD) {
			break;
		}
		ro += rd * closestScale * res.closest;
	}
	res.closest *= closestScale;
	res.dist = distance(start, ro);
	return res;
}

#endif
