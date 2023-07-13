#pragma once
#include "light_system.h"
#include <glm/gtx/rotate_vector.hpp>

#include <scorch/ecs/level.h>
#include <scorch/ecs/actor.h>
#include <scorch/ecs/components.h>

namespace ScorchEngine {
	void LightSystem::update(FrameInfo& frameInfo, SceneSSBO& sceneBuffer) {
		sceneBuffer.hasDirectionalLight = VK_FALSE;
		frameInfo.level->getRegistry().view<Components::TransformComponent, Components::DirectionalLightComponent>().each(
			[&](auto& transform, auto& light) {
				glm::mat4 tm = transform.getTransformMatrix();
				tm = glm::rotate(tm, glm::half_pi<float>(), glm::vec3(1.f, 0.f, 0.f));
				glm::vec3 direction = glm::normalize(tm[2]); // convert rotation to direction

				sceneBuffer.directionalLight.direction = direction;
				sceneBuffer.directionalLight.color = { light.emission, light.intensity };
				sceneBuffer.directionalLight.vp = light.shadow.vp;
				sceneBuffer.hasDirectionalLight = VK_TRUE;
			}
		);

		sceneBuffer.hasLPV = VK_FALSE;
		frameInfo.level->getRegistry().view<Components::TransformComponent, Components::LightPropagationVolumeComponent>().each(
			[&](auto& transform, auto& lpv) {
				sceneBuffer.lpv.boost = lpv.boost * powf(2, lpv.cascadeCount - 1);
				sceneBuffer.lpv.center = lpv.center;

				for (int i = 0; i < lpv.cascadeCount; i++) {
					glm::vec3 cascadeExtent = lpv.maxExtent / powf(2, lpv.cascadeCount - i - 1);
					sceneBuffer.lpv.cascades[i].extent = cascadeExtent;
					sceneBuffer.lpv.cascades[i].virtualPropagatedGridRedUVMin = lpv.cascades[i].virtualPropagatedGridRedUVMin;
					sceneBuffer.lpv.cascades[i].virtualPropagatedGridRedUVMax = lpv.cascades[i].virtualPropagatedGridRedUVMax;
					sceneBuffer.lpv.cascades[i].virtualPropagatedGridGreenUVMin = lpv.cascades[i].virtualPropagatedGridGreenUVMin;
					sceneBuffer.lpv.cascades[i].virtualPropagatedGridGreenUVMax = lpv.cascades[i].virtualPropagatedGridGreenUVMax;
					sceneBuffer.lpv.cascades[i].virtualPropagatedGridBlueUVMin = lpv.cascades[i].virtualPropagatedGridBlueUVMin;
					sceneBuffer.lpv.cascades[i].virtualPropagatedGridBlueUVMax = lpv.cascades[i].virtualPropagatedGridBlueUVMax;
				}
				sceneBuffer.lpv.cascadeCount = lpv.cascadeCount;
				sceneBuffer.hasLPV = VK_TRUE;
			}
		);

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