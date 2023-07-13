#ifndef SCENE_SSBO_GLSL
#define SCENE_SSBO_GLSL

struct SkyLight {
	vec4 tint; // w = intensity
	mat4 vfaovp;
};

struct DirectionalLight {
	vec3 direction;
	vec4 color; // w = intensity
	mat4 vp;
};
struct LPVCascadeData {
	vec3 extent;
	vec3 virtualPropagatedGridRedUVMin;
	vec3 virtualPropagatedGridRedUVMax;
	vec3 virtualPropagatedGridGreenUVMin;
	vec3 virtualPropagatedGridGreenUVMax;
	vec3 virtualPropagatedGridBlueUVMin;
	vec3 virtualPropagatedGridBlueUVMax;
};
struct LPV {
	vec3 center;
	vec3 boost;
	LPVCascadeData cascades[4];
	uint cascadeCount;
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