#pragma once

#include <glm/glm.hpp>

namespace ScorchEngine {
	inline namespace Components {
		struct LightPropagationVolumeComponent {
			glm::vec3 extent = glm::vec3(35.0);
			glm::vec3 center;
			glm::vec3 boost = glm::vec3(1.0);
		};
	}
}