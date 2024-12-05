#include "water_system.h"

#include <scorch/ecs/components.h>
#include <scorch/ecs/actor.h>
#include <scorch/systems/resource_system.h>
#include <glm/gtx/rotate_vector.hpp>

namespace ScorchEngine {
	struct RSMPush {
		glm::mat4 vp;
		glm::mat4 modelMatrix;
	};

	WaterSystem::WaterSystem(
		SEDevice& device, 
		SEDescriptorPool& descriptorPool,
		VkRenderPass renderPass, 
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
		VkSampleCountFlagBits msaaSamples
	) : seDevice(device), seDescriptorPool(descriptorPool) {

	}
	WaterSystem::~WaterSystem() {

	}

}