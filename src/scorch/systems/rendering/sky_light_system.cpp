#pragma once
#include "sky_light_system.h"

#include <scorch/ecs/level.h>
#include <scorch/ecs/actor.h>
#include <scorch/ecs/components.h>

#include <scorch/systems/resource_system.h>

namespace ScorchEngine {
	SkyLightSystem::SkyLightSystem(SEDevice& device, SEDescriptorPool& descriptorPool, uint32_t framesInFlight) : seDevice(device), seDescriptorPool(descriptorPool) {
		descriptorSet.resize(framesInFlight);
	}
	SkyLightSystem::~SkyLightSystem() {
		for (const auto& [cubemap, envMap] : envCubeToMap) {
			delete envMap; 
		}
	}
	void SkyLightSystem::update(FrameInfo& frameInfo, SETextureCube* skyLight) {
		if (envCubeToMap.find(skyLight) == envCubeToMap.end()) {
			envCubeToMap[skyLight] = new SEEnvironmentMap(seDevice, seDescriptorPool, skyLightDescriptorLayout, skyLight->getImageInfo(), { skyLight->getExtent().width, skyLight->getExtent().height }, false);
			VkCommandBuffer commandBuffer = seDevice.beginSingleTimeCommands();
			envCubeToMap[skyLight]->generateEnvironmentBRDF(commandBuffer);
			envCubeToMap[skyLight]->generateIrradianceMap(commandBuffer);
			envCubeToMap[skyLight]->generatePrefilteredEnvironmentMap(commandBuffer);
		} 
		descriptorSet[frameInfo.frameIndex] = envCubeToMap[skyLight]->getEnvironmentMapDescriptor();
	}
}