#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec3 ray;
layout (location = 1) in vec3 local;
layout (location = 2) in vec3 world;
layout (location = 0) out vec4 outColor;

#include "global_ubo.glsl"

layout (set = 1, binding = 0) uniform sampler3D sdf;

layout (push_constant) uniform Push {
	mat4 invTransform;
	vec3 halfExtent;
	vec3 scale;
} push;


float THRESHOLD = 0.015;
int MAX_STEPS = 64;
float EPSILON = 0.02;

mat3 rotationMatrix;

vec3 worldToVoxelSpace(vec3 p) {
	vec3 cc = (push.invTransform * vec4(p, 1.0)).xyz / (push.halfExtent);
	return cc * 0.5 + 0.5;
}

vec2 sceneSDF(vec3 ro) {
	vec3 start = ro;
	vec3 rd = normalize(ray);
	float closest = 1000000000.0;
	//float closestScale = dot(push.scale, abs(rotationMatrix * rd));
	float closestScale =  min(push.scale.x, min(push.scale.y, push.scale.z));
	for (int i = 0; i < MAX_STEPS; ++i) {
		vec3 uv = worldToVoxelSpace(ro);
		closest = closestScale * texture(sdf, uv).r;
		if (closest < THRESHOLD) {
			break;
		}
		ro += rd * closest;
	}
	return vec2(closest, distance(start, ro));
}

vec3 estimateNormal(vec3 p) {
    return normalize(vec3(
        sceneSDF(vec3(p.x + EPSILON, p.y, p.z)).y - sceneSDF(vec3(p.x - EPSILON, p.y, p.z)).y,
        sceneSDF(vec3(p.x, p.y + EPSILON, p.z)).y - sceneSDF(vec3(p.x, p.y - EPSILON, p.z)).y,
        sceneSDF(vec3(p.x, p.y, p.z  + EPSILON)).y - sceneSDF(vec3(p.x, p.y, p.z - EPSILON)).y
    ));
}


void main() {
	mat4 transformMatrix = inverse(push.invTransform);
	rotationMatrix = mat3(
		normalize(transformMatrix[0].xyz),
		normalize(transformMatrix[1].xyz),
		normalize(transformMatrix[2].xyz)
	);

	vec3 ro = world;
	vec2 res = sceneSDF(world);
	if (res.x > THRESHOLD) {
		discard;
	}
	vec3 nor = estimateNormal(world);
	vec3 dir = vec3(-1.0, -1.0, 0.5);
	vec3 col = max(dot(nor, -dir), 0.0) * vec3(1.0) + vec3(0.2, 0.25, 0.3);
	outColor = vec4(col, 1.0);

	vec3 trueWorld = world + normalize(ray) * res.y;
	vec4 clip = ubo.viewProjMatrix * vec4(trueWorld, 1.0);
	gl_FragDepth = clip.z / clip.w;
}