#pragma once
#include "sky_light_system.h"

#include <scorch/ecs/level.h>
#include <scorch/ecs/actor.h>
#include <scorch/ecs/components.h>

#include <scorch/systems/resource_system.h>

namespace ScorchEngine {
	SkyLightSystem::SkyLightSystem(SEDevice& device, std::unique_ptr<SEDescriptorSetLayout>& skyLightDescriptorLayout, uint32_t framesInFlight)
		: seDevice(device), skyLightDescriptorLayout(skyLightDescriptorLayout) {
		descriptorSet.resize(framesInFlight);
	}
	SkyLightSystem::~SkyLightSystem() {
		for (const auto& [cubemap, envMap] : envCubeToMap) {
			delete envMap; 
		}
	}
	void SkyLightSystem::update(FrameInfo& frameInfo, SceneSSBO& sceneBuffer, SETextureCube* skyLight) {
		if (envCubeToMap.find(skyLight) == envCubeToMap.end()) {
			envCubeToMap[skyLight] = new SEEnvironmentMap(seDevice, seDevice.getDescriptorPool(), skyLightDescriptorLayout, skyLight->getImageInfo(), { skyLight->getExtent().width, skyLight->getExtent().height }, false);
			VkCommandBuffer commandBuffer = seDevice.beginSingleTimeCommands();
			envCubeToMap[skyLight]->generateEnvironmentBRDF(commandBuffer);
			envCubeToMap[skyLight]->generateIrradianceMap(commandBuffer);
			envCubeToMap[skyLight]->generatePrefilteredEnvironmentMap(commandBuffer);
			seDevice.endSingleTimeCommands(commandBuffer);
		} 
		descriptorSet[frameInfo.frameIndex] = envCubeToMap[skyLight]->getEnvironmentMapDescriptor();
		

		sceneBuffer.hasSkyLight = VK_FALSE;
		frameInfo.level->getRegistry().view<Components::SkyLightComponent>().each(
			[&](auto& skylight) {
			sceneBuffer.skyLight.tint = { skylight.tint, skylight.intensity };
			sceneBuffer.skyLight.vfaovp = skylight.vfao.vp;
			sceneBuffer.hasSkyLight = VK_TRUE;
		}
		);
	}
}