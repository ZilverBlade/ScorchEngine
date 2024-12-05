#version 450
layout (location = 0) in vec2 fragUV;

layout (location = 0) out float WaterHeightMap;
layout (location = 1) out vec2 WaterNormalMap;
layout (location = 2) out float WaterSpecularMap;
layout (location = 3) out vec4 WaterDiffuseMap;
layout (location = 4) out float WaterCausticMap;

layout (push_constant) uniform Push {
	float elapsed_ty;
} push;

void main() {
	
}