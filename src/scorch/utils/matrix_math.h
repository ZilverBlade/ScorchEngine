#pragma once

#include <cstdint>
namespace ScorchEngine {
    namespace Math {
        glm::mat4 calculateYXZ(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale) {
			const float c3 = glm::cos(rotation.y);
			const float s3 = glm::sin(rotation.y);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.z);
			const float s1 = glm::sin(rotation.z);
			return glm::mat4(
				{
					scale.x * (c1 * c3 + s1 * s2 * s3),
					scale.x * (c2 * s3),
					scale.x * (c1 * s2 * s3 - c3 * s1),
					0.0f,
				},
			{
				scale.z * (c3 * s1 * s2 - c1 * s3),
				scale.z * (c2 * c3),
				scale.z * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				scale.y * (c2 * s1),
				scale.y * (-s2),
				scale.y * (c1 * c2),
				0.0f,
			},
				{ translation.x, translation.z, translation.y, 1.0f }
			);
        }
    }
}
