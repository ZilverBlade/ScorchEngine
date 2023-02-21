#pragma once

#include <glm/glm.hpp>

namespace ScorchEngine {
	inline namespace Components {
		struct SkyboxComponent {
			glm::vec3 tint{ 1.0f };
			float intensity{ 1.0f };
			glm::vec3 envTint{ 1.0f };
			float envIntensity{ 1.0f };
			ResourceID environmentMap{};
		};
	}
}