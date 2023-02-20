#pragma once

#include <glm/glm.hpp>

namespace ScorchEngine {
	struct DirectionalLight {
		alignas(16)glm::vec3 direction;
		alignas(16)glm::vec4 color; // w = intensity
	};
	struct PointLight {
		alignas(16)glm::vec3 position;
		alignas(16)glm::vec4 color; // w = intensity
	};
	struct SceneSSBO {
		DirectionalLight directionalLights[16];
		PointLight pointLights[512];
		uint32_t directionalLightCount;
		uint32_t pointLightCount;
	};
}