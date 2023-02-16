#pragma once

#include <vulkan/vulkan.h>

namespace ScorchEngine {
	struct FrameInfo {
		VkCommandBuffer commandBuffer;
		uint32_t frameIndex;
		VkDescriptorSet globalUBO;
		VkDescriptorSet sceneSSBO;
	};
}