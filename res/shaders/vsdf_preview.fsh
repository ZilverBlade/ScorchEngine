#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec3 ray;
layout (location = 0) out vec4 outColor;

#include "global_ubo.glsl"

layout (set = 1, binding = 0) uniform sampler3D sdf;

layout (push_constant) uniform Push {
	mat4 inverseTransform;
	vec3 halfExtent;
} push;


vec3 worldToVoxelSpace(vec3 p) {
	vec3 cc = (push.inverseTransform * vec4(p, 1.0)).xyz / push.halfExtent;
	return cc * 0.5 + 0.5;
}

float THRESHOLD = 0.001;
int MAX_STEPS = 8;

void main() {
	vec3 ro = ubo.invViewMatrix[3].xyz;
	vec3 rd = normalize(ray);

	bool hit = false;
	for (int i = 0; i < MAX_STEPS; ++i) {
		float closest = texture(sdf, worldToVoxelSpace(ro)).r;
		if (closest < THRESHOLD) {
			hit = true;
			break;
		}
		ro += rd * closest;
	}

	if (hit) {
		vec3 nor = normalize(cross(dFdx(ro) - ro, dFdy(ro) - ro));
		float col = dot(rd, nor);
		outColor = vec4(col.xxx, 1.0);
	} else {
		outColor = vec4(0..xxx, 1.0);
	}
}