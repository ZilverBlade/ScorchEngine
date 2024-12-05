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

//https://gist.github.com/wwwtyro/beecc31d65d1004f5a9d
float raySphereIntersect(vec3 r0, vec3 rd, vec3 s0, float sr) {
    // - r0: ray origin
    // - rd: normalized ray direction
    // - s0: sphere center
    // - sr: sphere radius
    // - Returns distance from r0 to first intersecion with sphere,
    //   or -1.0 if no intersection.
    float a = dot(rd, rd);
    vec3 s0_r0 = r0 - s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sr * sr);
    if (b*b - 4.0*a*c < 0.0) {
        return -1.0;
    }
    return (-b - sqrt((b*b) - 4.0*a*c))/(2.0*a);
}

const float bias  = SDF_THRESHOLD + 0.5;
void main() {
	float depth = textureLod(depthTexture, fragUV, 0.0).r;
	if (depth == 1.0) discard;
	vec4 cc = vec4(fragUV * 2.0 - 1.0, depth, 1.0);
	vec4 viewSpace = ubo.invProjMatrix * cc;
	viewSpace.xyz /= viewSpace.w;
	vec4 world = ubo.invViewMatrix * viewSpace;
	vec3 nor = normalize(cross(dFdy(world.xyz), dFdx(world.xyz)));
	world.xyz += nor * bias;
	
	float cScale = min(push.scale.x, min(push.scale.y, push.scale.z));
	
	mat4 transform = inverse(push.invTransform);
	vec3 center = transform[3].xyz;
	float radius = length(push.scale * push.halfExtent);
	
	float d = raySphereIntersect(world.xyz, push.toSun, center, radius);
	if (d < 0.0) discard;
	world.xyz += nor * d;
	
	SdfResult res = sceneSdf(push.toSun, world.xyz, push.invTransform, push.halfExtent, cScale);
	outVisiblity = res.closest > 0.0001 ? 1.0 : 0.0;
}