#ifndef CUBEMAP_UTILS_GLSL
#define CUBEMAP_UTILS_GLSL
const uint FACE_INDEX_posX = 0;
const uint FACE_INDEX_negX = 1;
const uint FACE_INDEX_posY = 2;
const uint FACE_INDEX_negY = 3;
const uint FACE_INDEX_posZ = 4;
const uint FACE_INDEX_negZ = 5;

vec3 getCubeUV(vec2 inUV, uint faceIndex) {
	vec2 realUV = inUV * 2.0 - 1.0;
 	if (faceIndex == FACE_INDEX_posX) 
		return vec3(1.0, realUV.y, realUV.x);
	if (faceIndex == FACE_INDEX_negX) 
		return vec3(-1.0, realUV.y, realUV.x);
 	if (faceIndex == FACE_INDEX_posY) 
		return vec3(realUV.x, 1.0, realUV.y);
	if (faceIndex == FACE_INDEX_negY) 
		return vec3(realUV.x, -1.0, realUV.y);
 	if (faceIndex == FACE_INDEX_posZ) 
		return vec3(realUV.x, realUV.y, 1.0);
	if (faceIndex == FACE_INDEX_negZ) 
		return vec3(realUV.x, realUV.y, -1.0);
}
#endif