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
	float roughness;
} push;

void main() {
	const vec3 N = normalize(vec3(fragUV.x, -fragUV.y, fragUV.z));
    const vec3 R = N;
    const vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0;   
    vec3 prefilteredColor = vec3(0.0);     
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = hammersley(i, SAMPLE_COUNT);
        vec3 H  = importanceSampleGGX(Xi, N, push.roughness * push.roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            prefilteredColor += texture(environmentMap, L).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }
    outDiffuse = vec4(prefilteredColor / totalWeight, 1.0);
	
}