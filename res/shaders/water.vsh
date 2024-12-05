#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec3 position;

layout (location = 0) out vec2 fragPosWorld;
layout (location = 1) out vec2 fragUV;
layout (location = 2) out vec3 fragNormal;
layout (location = 3) out vec3 fragTangent;

layout (set = 4, binding = 0) uniform sampler2D WaterHeightMap;
layout (set = 4, binding = 1) uniform sampler2D WaterNormalMap;
layout (set = 4, binding = 2) uniform sampler2D WaterSpecularMap;
layout (set = 4, binding = 3) uniform sampler2D WaterMaterialMap;
 
layout (push_constant) uniform Push {
	mat4 vp;
	mat4 modelMatrix;
} push;

vec3 NORMAL = vec3(0.0, 1.0, 0.0);

void main() {
	vec3 truePosition = position;
	truePosition.y = textureLod(WaterHeightMap, position.xz);
	
	gl_Position = push.vp * push.modelMatrix * vec4(truePosition, 1.0);
	fragNormal = NORMAL; 
}