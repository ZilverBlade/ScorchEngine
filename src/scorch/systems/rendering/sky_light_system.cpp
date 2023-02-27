#pragma once
#include "sky_light_system.h"

#include <scorch/ecs/level.h>
#include <scorch/ecs/actor.h>
#include <scorch/ecs/components.h>

#include <scorch/systems/resource_system.h>

namespace ScorchEngine {
	SkyLightSystem::SkyLightSystem(SEDevice& device, SEDescriptorPool& descriptorPool, uint32_t framesInFlight) : seDevice(device), seDescriptorPool(descriptorPool) {
		

	}
	SkyLightSystem::~SkyLightSystem() {
	}
	void SkyLightSystem::update(SESkyLight* skyLight) {}
}