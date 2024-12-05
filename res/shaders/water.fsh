#version 450
#extension GL_GOOGLE_include_directive : enable
#include "normal_mapping.glsl"
#include "pbr.glsl"

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 fragPosWorld;
layout (location = 1) in vec2 fragUV;
layout (location = 2) in vec3 fragNormal;
layout (location = 3) in vec3 fragTangent;


layout (set = 4, binding = 0) uniform sampler2D WaterHeightMap;
layout (set = 4, binding = 1) uniform sampler2D WaterNormalMap;
layout (set = 4, binding = 2) uniform sampler2D WaterSpecularMap;
layout (set = 4, binding = 3) uniform sampler2D WaterDiffuseMap;

void main() {
	vec3 N = normalize(fragNormal);
	FragmentLitPBRData fragment;
	fragment.position = fragPosWorld;
	vec3 T = normalize(fragTangent);
	mat3 TBN = mat3(T, cross(N, T), N);

	sampleNormalMap(vec3(texture(WaterNormalMap, uv).xy, 1.0), TBN);
	fragment.diffuse = texture(WaterDiffuseMap, fragUV).rgb;
	fragment.specular =  texture(WaterSpecularMap, fragUV).x;
	fragment.roughness = 0.333;
	fragment.metallic = 0.981;
	fragment.ambientOcclusion = 1.0;
	
	FragmentClearCoatPBRData fragmentcc;
	fragmentcc.clearCoat = 0.0;
	vec3 lighting = pbrCalculateLighting(fragment, fragmentcc);
	outColor = vec4(lighting + emission,surfaceMaterialParams.opacity);
}