#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec3 ray;
layout (location = 1) in vec3 local;
layout (location = 2) in vec3 world;
layout (location = 0) out vec4 outColor;

#include "global_ubo.glsl"
#include "vsdf_common.glsl"

layout (push_constant) uniform Push {
	mat4 invTransform;
	vec3 halfExtent;
	vec3 scale;
} push;

float EPSILON = 0.02;
float cScale;

vec3 estimateNormal(vec3 p) {
	vec3 rd = normalize(ray);
    return normalize(vec3(
        sceneSdf(rd, vec3(p.x + EPSILON, p.y, p.z), push.invTransform, push.halfExtent, cScale).dist - sceneSdf(rd, vec3(p.x - EPSILON, p.y, p.z), push.invTransform, push.halfExtent, cScale).dist,
        sceneSdf(rd, vec3(p.x, p.y + EPSILON, p.z), push.invTransform, push.halfExtent, cScale).dist - sceneSdf(rd, vec3(p.x, p.y - EPSILON, p.z), push.invTransform, push.halfExtent, cScale).dist,
        sceneSdf(rd, vec3(p.x, p.y, p.z  + EPSILON), push.invTransform, push.halfExtent, cScale).dist - sceneSdf(rd, vec3(p.x, p.y, p.z - EPSILON), push.invTransform, push.halfExtent, cScale).dist
    ));
}


void main() {
	cScale = min(push.scale.x, min(push.scale.y, push.scale.z));
	vec3 rd = normalize(ray);
	SdfResult res = sceneSdf(rd, world, push.invTransform, push.halfExtent, cScale);
	if (res.closest > SDF_THRESHOLD) {
		discard;
	}
	vec3 nor = estimateNormal(world);
	vec3 dir = vec3(-1.0, -1.0, 0.5);
	vec3 col = max(dot(nor, -dir), 0.0) * vec3(0.7) + vec3(0.2, 0.25, 0.3);
	outColor = vec4(col, 1.0);

	vec3 trueWorld = world + normalize(ray) * res.dist;
	vec4 clip = ubo.viewProjMatrix * vec4(trueWorld, 1.0);
	gl_FragDepth = clip.z / clip.w;
}