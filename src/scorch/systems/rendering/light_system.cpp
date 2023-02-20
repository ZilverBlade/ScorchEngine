#pragma once
#include "light_system.h"
#include <glm/gtx/rotate_vector.hpp>

#include <scorch/ecs/level.h>
#include <scorch/ecs/actor.h>
#include <scorch/ecs/components.h>

namespace ScorchEngine {
	void LightSystem::update(FrameInfo& frameInfo, SceneSSBO& sceneBuffer) {
		uint32_t directionalLightIndex = 0;
		frameInfo.level->getRegistry().view<Components::TransformComponent, Components::DirectionalLightComponent>().each(
			[&](auto& transform, auto& light) {
				glm::mat4 tm = transform.getTransformMatrix();
				tm = glm::rotate(tm, glm::half_pi<float>(), glm::vec3(1.f, 0.f, 0.f));
				glm::vec3 direction = glm::normalize(tm[2]); // convert rotation to direction

				sceneBuffer.directionalLights[directionalLightIndex].direction = direction;
				sceneBuffer.directionalLights[directionalLightIndex].color = { light.emission, light.intensity };
				directionalLightIndex++;
			}
		);
		sceneBuffer.directionalLightCount = directionalLightIndex;

		uint32_t pointLightIndex = 0;
		frameInfo.level->getRegistry().view<Components::TransformComponent, Components::PointLightComponent>().each(
			[&](auto& transform, auto& light) {
				glm::mat4 tm = transform.getTransformMatrix();
				sceneBuffer.pointLights[pointLightIndex].position = glm::vec3(tm[3]);
				sceneBuffer.pointLights[pointLightIndex].color = { light.emission, light.intensity };
				pointLightIndex++;
			}
		);
		sceneBuffer.pointLightCount = pointLightIndex;
	}
}