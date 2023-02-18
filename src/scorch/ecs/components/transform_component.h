#pragma once

#include <string>
#include <scorch/utils/uuid.h>
#include <glm/glm.hpp>

namespace ScorchEngine {
	inline namespace Components {
		struct TransformComponent {
			glm::vec3 translation{ 0.0f, 0.0f, 0.0f };
			glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
			glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

			glm::mat4 getTransformMatrix();
			glm::mat3 getNormalMatrix();
		};
	}
}