#pragma once
#include "light_system.h"
#include <glm/gtx/rotate_vector.hpp>

#include <scorch/ecs/level.h>
#include <scorch/ecs/actor.h>
#include <scorch/ecs/components.h>

namespace ScorchEngine {
	void LightSystem::update(FrameInfo& frameInfo, SceneSSBO& sceneBuffer) {
		uint32_t i = 0;
		frameInfo.level->getRegistry().view<Components::TransformComponent, Components::DirectionalLightComponent>().each(
			[&](auto& transform, auto& light) {
				glm::mat4 tm = transform.getTransformMatrix();
				tm = glm::rotate(tm, glm::half_pi<float>(), glm::vec3(1.f, 0.f, 0.f));
				glm::vec3 direction = glm::normalize(tm[2]); // convert rotation to direction

				sceneBuffer.directionalLights[i].direction = -direction; // direction points away for some reason, so this has to be inverted
				sceneBuffer.directionalLights[i].color = { light.emission, light.intensity };
				i++;
			}
		);
		sceneBuffer.directionalLightCount = i;
	}
}