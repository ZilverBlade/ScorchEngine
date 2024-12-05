#version 450
#extension GL_GOOGLE_include_directive : enable
#include "global_ubo.glsl"

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;

layout (location = 0) out vec3 fragPosWorld;
layout (location = 1) out vec2 fragUV;
layout (location = 2) out vec3 fragNormal;
layout (location = 3) out vec3 fragTangent;

layout (push_constant) uniform Push {
	vec4 minBounds;
	vec4 maxBounds;
	vec4 minScale;
	vec4 maxScale;
	vec4 windDirection;
	uint seed;
	float elapsed;
} push;

uint hash( uint seed ) {
    seed += ( seed << 10u );
    seed ^= ( seed >>  6u );
    seed += ( seed <<  3u );
    seed ^= ( seed >> 11u );
    seed += ( seed << 15u );
    return seed;
}
float rand( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

mat3 createRotation(uint seed) {
	float rotate = rand(hash(hash(seed))) * 3.1415926 * 2.0;
	float ySin = sin(rotate);
	float yCos = cos(rotate);
	
	return mat3(
		vec3(yCos, 0.0, -ySin),
		vec3(0.0, 1.0, 0.0),
		vec3(ySin, 0.0, yCos)
	);
}

const float freqDiv = 2.50;
const float freqLift = 5.20;
const float mbfreq = 0.50;
const float freqA = 1.12;
const float freqB = 0.40;
const float freqC = 1.80;
const float mfreqD = 0.25;
const float nfreqD = 3.20;
const float dfreqD = 2.00;

void main() {
	uint seed = gl_InstanceIndex + push.seed;
	float alpha0 = rand(hash(seed) + gl_InstanceIndex);
	float alpha1 = rand(hash(-hash(seed))+ gl_InstanceIndex);
	float alpha2 = rand(hash(-hash(seed) * hash(seed)) + gl_InstanceIndex);
	vec3 vAlpha = vec3(alpha0, alpha1, alpha2);
	
	vec3 scaleOff = mix(push.minScale.xyz, push.maxScale.xyz, vAlpha);
	vec3 posOff = mix(push.minBounds.xyz, push.maxBounds.xyz, vAlpha);
	mat3 rotOff = createRotation(seed);
	
	vec3 worldPosition = rotOff * (position * scaleOff) + posOff; 
	
	// wind effect
	float wf = clamp(smoothstep(0.0, 1.0, max(pow((max(worldPosition - posOff, 0.0)).y, 2.0), 0.0)), 0.0, 1.0) * 0.2;
	float bPhase = push.elapsed * 1.8 + alpha0 * 23.52162;
	float wi = freqLift + mbfreq * sin(bPhase + bPhase * wf) + sin(freqA * bPhase) + cos(freqB * bPhase) + sin(freqC * bPhase)+ mfreqD * nfreqD * tan(bPhase) * cos (bPhase) * dfreqD;
	wi /= freqDiv; 
	worldPosition.xyz += push.windDirection.xyz * wi * wf * vAlpha;
	
	gl_Position = ubo.viewProjMatrix *vec4(worldPosition, 1.0);
	
	fragPosWorld = worldPosition.xyz;
	fragUV = uv;
	
	mat3 model3x3 = mat3(rotOff[0] * scaleOff.x, rotOff[1] * scaleOff.y, rotOff[2] * scaleOff.z);
	mat3 normalMatrix = inverse(transpose(model3x3));
	fragNormal = normalize(normalMatrix * normal); 
	fragTangent = normalize(normalMatrix * tangent);
}