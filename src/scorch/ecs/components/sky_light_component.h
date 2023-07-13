#pragma once

#include <glm/glm.hpp>
#include <scorch/utils/resid.h>

namespace ScorchEngine {
	inline namespace Components {
		struct SkyLightComponent {
			glm::vec3 tint{ 1.0f };
			float intensity{ 1.0f };
			ResourceID environmentMap{};
			struct VFAO {
				float slopeRadius = 5.0f;
				float slopeSlopeCompare = 0.25f;
				float bounds = 50.0f;
				glm::mat4 vp;
			} vfao;
		};
	}
}