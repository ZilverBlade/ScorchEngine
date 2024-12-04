#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec3 ray;
layout (location = 1) in vec3 local;
layout (location = 2) in vec3 world;
layout (location = 0) out vec4 outColor;

#include "global_ubo.glsl"

layout (set = 1, binding = 0) uniform sampler3D sdf;

layout (push_constant) uniform Push {
	vec4 translation;
	vec3 halfExtent;
} push;


float THRESHOLD = 0.01;
int MAX_STEPS = 32;
float EPSILON = 0.004;

vec3 worldToVoxelSpace(vec3 p) {
	vec3 cc = (p - push.translation.xyz) / (push.halfExtent / 2.0);
	return cc * 0.5 + 0.5;
}

vec2 sceneSDF(vec3 ro) {
	vec3 start = ro;
	vec3 rd = normalize(ray);
	float closest = 1000000000.0;
	for (int i = 0; i < MAX_STEPS; ++i) {
		vec3 uv = worldToVoxelSpace(ro);
		closest = texture(sdf, uv).r;
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
	vec3 ro = world;
	float closest = sceneSDF(world).x;
	if (closest > THRESHOLD) {
		discard;
	}
	vec3 nor = estimateNormal(world);
	vec3 dir = vec3(-1.0, -1.0, 0.5);
	vec3 col = clamp(dot(nor, -dir) * vec3(1.0), 0..xxx, 1..xxx);
	outColor = vec4(col, 1.0);

}