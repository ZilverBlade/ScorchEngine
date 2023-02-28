#version 450

layout (location = 0) in vec2 fragUV;
layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform sampler2D inputImage;

layout (push_constant) uniform Push {
	float ditherIntensity;
	float invGamma;
} push;

float random(vec2 coords) {
   return fract(sin(dot(coords.xy, vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
	vec2 flippedCoord = vec2(fragUV.x, 1.0 - fragUV.y);

	vec3 color = textureLod(inputImage, flippedCoord, 0.0).rgb;

	//color.r += mix(-push.ditherIntensity, push.ditherIntensity, random(gl_FragCoord.xy));
	//color.g += mix(-push.ditherIntensity, push.ditherIntensity, random(gl_FragCoord.xy));
	//color.b += mix(-push.ditherIntensity, push.ditherIntensity, random(gl_FragCoord.xy));
	
	vec3 gammaCorrected = pow(color, vec3(push.invGamma));
	
	outColor = vec4(gammaCorrected, 1.0);
}