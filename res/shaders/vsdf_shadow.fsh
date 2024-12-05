#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec2 fragUV;
layout (location = 0) out float outVisiblity;

#include "global_ubo.glsl"
#include "vsdf_common.glsl"

layout (set = 2, binding = 0) uniform sampler2D depthTexture;

layout (push_constant) uniform Push {
	mat4 invTransform;
	vec3 halfExtent;
	vec3 scale;
	vec3 toSun;
} push;

void main() {
	float depth = textureLod(depthTexture, fragUV, 0.0).r;
	if (depth == 1.0) discard;
	vec4 cc = vec4(fragUV * 2.0 - 1.0, depth, 1.0);
	vec4 viewSpace = ubo.invProjMatrix * cc;
	viewSpace.xyz /= viewSpace.w;
	vec4 world = ubo.invViewMatrix * viewSpace;

	float cScale = min(push.scale.x, min(push.scale.y, push.scale.z));

	SdfResult res = sceneSdf(push.toSun, world.xyz, push.invTransform, push.halfExtent, cScale);
	outVisiblity = step(res.closest, SDF_THRESHOLD);
}