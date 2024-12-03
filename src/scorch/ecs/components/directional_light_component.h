#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace ScorchEngine {
	inline namespace Components {
		struct DirectionalLightComponent {
			struct ShadowMap {
				float maxDistance = 35.0f;
				glm::mat4 vp{1.f};
			} shadow;
			glm::vec3 emission{1.0f};
			float intensity{1.0f};
		};
	}
}