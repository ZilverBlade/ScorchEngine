#pragma once

#include <vulkan/vulkan.h>
#include <scorch/ecs/level.h>

namespace ScorchEngine {
	class ResourceSystem;
	struct FrameInfo {
		VkCommandBuffer commandBuffer;
		uint32_t frameIndex;
		float frameTime;
		VkDescriptorSet globalUBO;
		VkDescriptorSet sceneSSBO;
		VkDescriptorSet skyLight;
		std::shared_ptr<Level> level;
		ResourceSystem* resourceSystem;
	};
}