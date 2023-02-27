#version 450

layout (location = 0) in vec2 fragUV;
layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform sampler2D inputImage;

layout (push_constant) uniform Push {
    vec2 screenSize;
	vec2 direction;
} push;

void main()	{
    vec3 blurColor = vec3(0.0);
    vec2 texelSize = 1.0 / push.screenSize;
	vec2 off1 = vec2(1.2846153846) * push.direction;
	vec2 off2 = vec2(2.5307692308) * push.direction;
	blurColor += textureLod(sceneInputImage, fragUV						, 0.0).rgb * 0.2270270270;
	blurColor += textureLod(sceneInputImage, fragUV + off1 * texelSizeX	, 0.0).rgb * 0.3162162162;
	blurColor += textureLod(sceneInputImage, fragUV - off1 * texelSizeX	, 0.0).rgb * 0.3162162162;
	blurColor += textureLod(sceneInputImage, fragUV + off2 * texelSizeX	, 0.0).rgb * 0.0702702703;
	blurColor += textureLod(sceneInputImage, fragUV - off2 * texelSizeX	, 0.0).rgb * 0.0702702703;
  
    outColor = vec4(blurColor, 1.0);
}