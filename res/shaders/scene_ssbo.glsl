#ifndef SCENE_SSBO_GLSL
#define SCENE_SSBO_GLSL

struct Skybox {
	vec4 tint;
	vec4 envTint;
};

struct DirectionalLight {
	vec3 direction;
	vec4 color; // w = intensity
};
struct PointLight {
	vec3 position;
	vec4 color; // w = intensity
};

layout (set = 1, binding = 0) readonly buffer SceneSSBO {
	Skybox skybox;
	DirectionalLight directionalLights[16];
	PointLight pointLights[512];
	uint directionalLightCount;
	uint pointLightCount;
} scene;
#endif