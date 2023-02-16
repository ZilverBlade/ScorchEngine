#pragma once

#include <glm/glm.hpp>

namespace ScorchEngine {
	struct GlobalUBO {
		glm::mat4 viewMatrix;
		glm::mat4 invViewMatrix;
		glm::mat4 projMatrix;
		glm::mat4 invProjMatrix;

		glm::mat4 viewProjMatrix;

		glm::vec2 screenSize;
		float gamma;
		float aspectRatio;
	};
}