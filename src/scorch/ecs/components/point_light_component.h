#pragma once

#include <glm/glm.hpp>

namespace ScorchEngine {
	inline namespace Components {
		struct PointLightComponent {
			glm::vec3 emission{1.0f};
			float intensity{1.0f};
		};
	}
}