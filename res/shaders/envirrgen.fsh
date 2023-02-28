#version 450
#extension GL_GOOGLE_include_directive : enable
#include "pbr_math.glsl"
#include "constants.glsl"
#include "magic.glsl"
#include "cubemap_utils.glsl"

layout (set = 0, binding = 0) uniform samplerCube environmentMap;

layout (location = 0) in vec3 fragUV;
layout (location = 0) out vec4 outDiffuse;

layout (push_constant) uniform Push {
	mat4 mvp;
} push;

// use this for static cubemap generation for correct results, otherwise for dynamic reflections use the faster gaussian blur approximation

void main() {
	const vec3 N = normalize(vec3(fragUV.x, -fragUV.y, fragUV.z));
	
	const vec3 up = vec3(0.0, 1.0, 0.0);
	const vec3 right = normalize(cross(up, N));
	
	vec3 irradiance = vec3(0.0); 
	
	const float sampleDelta = 0.025;
	float nrSamples = 0.0; 
	for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
		for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
			// spherical to cartesian (in tangent space)
			vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 
	
			irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}
	outDiffuse = vec4(PI * irradiance / float(nrSamples), 1.0);
}