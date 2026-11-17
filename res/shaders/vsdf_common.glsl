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
	float visibility;
};

vec3 worldToVoxelSpace(vec3 p, mat4 inverseTransform, vec3 sdfHalfExtent) {
	vec3 cc = (inverseTransform * vec4(p, 1.0)).xyz / sdfHalfExtent;
	return cc * 0.5 + 0.5;
}

SdfResult sceneSdf(vec3 rd, vec3 ro, mat4 inverseTransform, vec3 sdfHalfExtent, float closestScale) {
	vec3 start = ro;
	SdfResult res;
	res.visibility = 1.0;
	//float closestScale = dot(push.scale, abs(rotationMatrix * rd));
	for (int i = 0; i < SDF_MAX_STEPS; ++i) {
		vec3 uv = worldToVoxelSpace(ro, inverseTransform, sdfHalfExtent);
		res.closest = texture(sdf, uv).r;
		if (res.closest < SDF_THRESHOLD) {
			res.visibility = 0.0;
			break;
		}
		ro += rd * closestScale * res.closest;
	}
	res.closest *= closestScale;
	res.dist = distance(start, ro);
	return res;
}


SdfResult sceneSdfSmooth(vec3 rd, vec3 ro, float k, mat4 inverseTransform, vec3 sdfHalfExtent, float closestScale) {
	vec3 start = ro;
	SdfResult res;
	res.dist = 0.0;
	res.visibility = 1.0;
	//float closestScale = dot(push.scale, abs(rotationMatrix * rd));
	for (int i = 0; i < SDF_MAX_STEPS; ++i) {
		vec3 uv = worldToVoxelSpace(ro, inverseTransform, sdfHalfExtent);
		res.closest = texture(sdf, uv).r;
		if (res.closest < SDF_THRESHOLD) {
        	res.visibility = 0.0;
			break;
		}
		float trueClosest = closestScale*res.closest;
        res.visibility = min( res.visibility, k*trueClosest/res.dist );
		ro += rd * trueClosest;
		res.dist += trueClosest;
	}
	res.closest *= closestScale;
	return res;
}


#endif
