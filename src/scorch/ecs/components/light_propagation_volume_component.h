#pragma once

#include <glm/glm.hpp>

namespace ScorchEngine {
	inline namespace Components {
		struct LightPropagationVolumeComponent {
			glm::vec3 maxExtent;
			glm::vec3 center;
			glm::vec3 boost = glm::vec3(1.0);
			int cascadeCount = 1;
			int propagationIterations = 2;
			struct LPVCascadeData {
				glm::vec3 virtualPropagatedGridRedUVMin;
				glm::vec3 virtualPropagatedGridRedUVMax;
				glm::vec3 virtualPropagatedGridGreenUVMin;
				glm::vec3 virtualPropagatedGridGreenUVMax;
				glm::vec3 virtualPropagatedGridBlueUVMin;
				glm::vec3 virtualPropagatedGridBlueUVMax;
			} cascades[4];
		};
	}
}