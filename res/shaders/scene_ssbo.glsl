#ifndef SCENE_SSBO_GLSL
#define SCENE_SSBO_GLSL

struct SkyLight {
	vec4 tint; // w = intensity
};

struct DirectionalLight {
	vec3 direction;
	vec4 color; // w = intensity
	mat4 vp;
};
struct LPV {
	vec3 center;
	vec3 extent;
	vec3 boost;
};
struct PointLight {
	vec3 position;
	vec4 color; // w = intensity
};

layout (set = 1, binding = 0) readonly buffer SceneSSBO {
	SkyLight skyLight;
	DirectionalLight directionalLight;
	LPV lpv;
	PointLight pointLights[128];
	bool hasSkyLight;
	bool hasDirectionalLight;
	bool hasLPV;
	uint pointLightCount;
} scene;
#endif