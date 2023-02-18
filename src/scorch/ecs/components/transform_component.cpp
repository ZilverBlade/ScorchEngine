#include "transform_component.h"
#include <scorch/utils/matrix_math.h>

namespace ScorchEngine::Components {
	// roll first, then pitch, then yaw (YXZ)
	glm::mat4 TransformComponent::getTransformMatrix() {
		return Math::calculateYXZ(translation, rotation, scale);
	}
	
	glm::mat3 TransformComponent::getNormalMatrix() {
		const float c3 = glm::cos(rotation.y);
		const float s3 = glm::sin(rotation.y);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.z);
		const float s1 = glm::sin(rotation.z);

		glm::vec3 invScale = 1.0f / scale;
		return glm::mat3(
			{
				invScale.x * (c1 * c3 + s1 * s2 * s3),
				invScale.x * (c2 * s3),
				invScale.x * (c1 * s2 * s3 - c3 * s1),
			},
			{
				invScale.z * (c3 * s1 * s2 - c1 * s3),
				invScale.z * (c2 * c3),
				invScale.z * (c1 * c3 * s2 + s1 * s3),
			},
			{
				invScale.y * (c2 * s1),
				invScale.y * (-s2),
				invScale.y * (c1 * c2),
			}
		);
	}
}
