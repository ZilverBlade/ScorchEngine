#version 450

layout (location = 0) in vec2 fragUV;
layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform samplerCube inputImage;

const uint FACE_INDEX_posX = 0;
const uint FACE_INDEX_negX = 1;
const uint FACE_INDEX_posY = 2;
const uint FACE_INDEX_negY = 3;
const uint FACE_INDEX_posZ = 4;
const uint FACE_INDEX_negZ = 5;

layout (push_constant) uniform Push {
	uint faceIndex;
    vec2 faceSize;
	vec3 direction;
} push;

void main()	{
	vec2 realUV = fragUV * 2.0 - 1.0;
	vec3 sampleUV;
 	if (push.faceIndex == FACE_INDEX_posX) 
		sampleUV = vec3(1.0, realUV.y, realUV.x);
	if (push.faceIndex == FACE_INDEX_negX) 
		sampleUV = vec3(-1.0, realUV.y, realUV.x);
 	if (push.faceIndex == FACE_INDEX_posY) 
		sampleUV = vec3(realUV.x, 1.0, realUV.y);
	if (push.faceIndex == FACE_INDEX_negY) 
		sampleUV = vec3(realUV.x, -1.0, realUV.y);
 	if (push.faceIndex == FACE_INDEX_posZ) 
		sampleUV = vec3(realUV.x, realUV.y, 1.0);
	if (push.faceIndex == FACE_INDEX_negZ) 
		sampleUV = vec3(realUV.x, realUV.y, -1.0);
	

    vec3 blurColor = vec3(0.0);
    float texelSize = 1.0 / push.faceSize.x; // cubemaps must have equal face sizes
	vec3 off1 = vec3(1.2846153846) * push.direction;
	vec3 off2 = vec3(2.5307692308) * push.direction;
	blurColor += textureLod(sceneInputImage, sampleUV					, 0.0).rgb * 0.2270270270;
	blurColor += textureLod(sceneInputImage, sampleUV + off1 * texelSize, 0.0).rgb * 0.3162162162;
	blurColor += textureLod(sceneInputImage, sampleUV - off1 * texelSize, 0.0).rgb * 0.3162162162;
	blurColor += textureLod(sceneInputImage, sampleUV + off2 * texelSize, 0.0).rgb * 0.0702702703;
	blurColor += textureLod(sceneInputImage, sampleUV - off2 * texelSize, 0.0).rgb * 0.0702702703;
  
    outColor = vec4(blurColor, 1.0);
}