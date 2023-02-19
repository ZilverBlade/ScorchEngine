#version 450
#extension GL_GOOGLE_include_directive : enable
#include "pbr.glsl"

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 fragPosWorld;
layout (location = 1) in vec2 fragUV;
layout (location = 2) in vec3 fragNormal;
layout (location = 3) in vec3 fragTangent;

void main() {
	FragmentPBRData fragment;
	fragment.position = fragPosWorld;
	fragment.normal = normalize(fragNormal);
	fragment.diffuse = vec3(1.0, 1.0, 1.0);
	fragment.specular = 1.0;
	fragment.roughness = 0.11;
	fragment.metallic = 0.0;
	fragment.ambientOcclusion = 1.0;
	
	vec3 lighting = pbrCalculateLighting(fragment);
	outColor = vec4(lighting + vec3(0.1, 0.1, 0.1), 1.0);
}