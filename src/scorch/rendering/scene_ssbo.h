#pragma once

#include <glm/glm.hpp>

namespace ScorchEngine {
	struct SkyLight {
		alignas(16)glm::vec4 tint; // w = intensity
		alignas(16)glm::mat4 vfaovp;
	};
	struct DirectionalLight {
		alignas(16)glm::vec3 direction;
		alignas(16)glm::vec4 color; // w = intensity
		alignas(16)glm::mat4 vp;
	};

	struct LPVCascadeData {
		alignas(16)glm::vec3 extent;
		alignas(16)glm::vec3 virtualPropagatedGridRedUVMin;
		alignas(16)glm::vec3 virtualPropagatedGridRedUVMax;
		alignas(16)glm::vec3 virtualPropagatedGridGreenUVMin;
		alignas(16)glm::vec3 virtualPropagatedGridGreenUVMax;
		alignas(16)glm::vec3 virtualPropagatedGridBlueUVMin;
		alignas(16)glm::vec3 virtualPropagatedGridBlueUVMax;
	};
	struct LPV {
		alignas(16)glm::vec3 center;
		alignas(16)glm::vec3 boost;
		alignas(16)LPVCascadeData cascades[4];
		alignas(16)uint32_t cascadeCount;
	};
	struct PointLight {
		alignas(16)glm::vec3 position;
		alignas(16)glm::vec4 color; // w = intensity
	};

	struct SceneSSBO {
		SkyLight skyLight;
		DirectionalLight directionalLight;
		LPV lpv;
		PointLight pointLights[128];
		VkBool32 hasSkyLight;
		VkBool32 hasDirectionalLight;
		VkBool32 hasLPV;
		uint32_t pointLightCount;
	};
}