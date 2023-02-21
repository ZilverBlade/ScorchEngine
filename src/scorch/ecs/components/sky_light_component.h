#pragma once

#include <glm/glm.hpp>
#include <scorch/utils/resid.h>

namespace ScorchEngine {
	inline namespace Components {
		struct SkyLightComponent {
			glm::vec3 tint{ 1.0f };
			float intensity{ 1.0f };
			ResourceID environmentMap{};
		};
	}
}