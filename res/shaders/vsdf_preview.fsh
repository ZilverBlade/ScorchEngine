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
	float time;
} push;

vec3 worldToVoxelSpace(vec3 p) {
	vec3 cc = (p - push.translation.xyz) / push.halfExtent;
	return cc * 0.5 + 0.5;
}

float THRESHOLD = 0.01;
int MAX_STEPS = 32;

void main() {
	vec3 ro = world;
	vec3 rd = normalize(ray);

	vec3 nor = cross(dFdy(world), dFdx(world));

	bool hit = false;
	float stepLength = 0.01;
	float closest = 100000.0;
	/*for (int i = 0; i < MAX_STEPS; ++i) {
		vec3 uv = worldToVoxelSpace(ro);
		closest = min(closest, texture(sdf, uv).r);
		if (closest < 0.0) {
			break;
		}
		//if (closest < THRESHOLD) {
		//	hit = true;
		//	break;
		//}
		ro += rd * stepLength;
	}*/
vec3 debug = clamp(texture(sdf, vec3(local.x, push.time, local.z) * 0.5 + 0.5).rrr, 0.0, 1.0);
	outColor = vec4(debug, 1.0);
	
	if (debug.r > 0.0) {
		discard;
	}
	outColor = vec4(1.0);

}