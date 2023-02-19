#ifndef SCENE_SSBO_GLSL
#define SCENE_SSBO_GLSL
struct DirectionalLight {
	vec3 direction;
	vec4 color; // w = intensity
};
layout (set = 1, binding = 0) readonly buffer SceneSSBO {
	DirectionalLight directionalLights[16];
	uint directionalLightCount;
} scene;
#endif