#version 450

layout (location = 0) in vec2 fragUV;
layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform sampler2D inputImage;
layout (set = 0, binding = 1) uniform sampler2D colorGradingLUT;

layout (push_constant) uniform Push {
	float ditherIntensity;
	float invGamma;
} push;

float random(vec2 coords) {
   return fract(sin(dot(coords.xy, vec2(12.9898,78.233))) * 43758.5453);
}
//#define LUT_FLIP_Y
//vec3 lookup(vec3 color, sampler2D LUT) {
//    float blueColor = color.b * 63.0;
//
//    vec2 quad1;
//    quad1.y = floor(floor(blueColor) / 8.0);
//    quad1.x = floor(blueColor) - (quad1.y * 8.0);
//
//    vec2 quad2;
//    quad2.y = floor(ceil(blueColor) / 8.0);
//    quad2.x = ceil(blueColor) - (quad2.y * 8.0);
//
//    vec2 texPos1;
//    texPos1.x = (quad1.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * color.r);
//    texPos1.y = (quad1.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * color.g);
//
//    #ifdef LUT_FLIP_Y
//        texPos1.y = 1.0-texPos1.y;
//    #endif
//
//    vec2 texPos2;
//    texPos2.x = (quad2.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * color.r);
//    texPos2.y = (quad2.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * color.g);
//
//    #ifdef LUT_FLIP_Y
//        texPos2.y = 1.0-texPos2.y;
//    #endif
//
//    vec3 newColor1 = texture(LUT, texPos1).rgb;
//    vec3 newColor2 = texture(LUT, texPos2).rgb;
//
//    vec3 newColor = mix(newColor1, newColor2, fract(blueColor));
//    return newColor;
//}
//
//vec3 lookup2(vec3 color, sampler2D LUT) {
//    const float LUT_DIMS = 48.0;
//    const vec3 uv = color * ((LUT_DIMS - 1.0) / LUT_DIMS) + 0.5 / LUT_DIMS;
//
//    return tony_mc_mapface_lut.SampleLevel(sampler_linear_clamp, uv, 0);
//}


void main() {
	vec2 flippedCoord = vec2(fragUV.x, 1.0 - fragUV.y);

	vec3 hdr = textureLod(inputImage, flippedCoord, 0.0).rgb;
	
	// tonemap
	vec3 color = vec3(1.0) - exp(-hdr);

	//color = lookup(color, colorGradingLUT);
	
	color.r += mix(-push.ditherIntensity, push.ditherIntensity, random(gl_FragCoord.xy));
	color.g += mix(-push.ditherIntensity, push.ditherIntensity, random(gl_FragCoord.xy));
	color.b += mix(-push.ditherIntensity, push.ditherIntensity, random(gl_FragCoord.xy));
	
	vec3 gammaCorrected = pow(color, vec3(push.invGamma));
	
	outColor = vec4(gammaCorrected, 1.0);
}