#pragma once

#include <vulkan/vulkan.h>

namespace ScorchEngine {
	class SEPushConstant {
	public:
		SEPushConstant() = default;
		SEPushConstant(size_t size, VkPipelineStageFlags stages, uint32_t offset = 0);
		~SEPushConstant();
		void push(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, const void* data);
		inline VkPushConstantRange getRange() { return pushConstantRange; }
	private:
		VkPushConstantRange pushConstantRange{};
	};
}