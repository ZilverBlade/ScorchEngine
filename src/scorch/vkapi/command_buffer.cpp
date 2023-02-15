#include "command_buffer.h"

namespace ScorchEngine {
	SECommandBuffer::SECommandBuffer(SEDevice& device, CommandBufferLevel level) : commandPool(device.getCommandPool()), seDevice(device) {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;
		if (level == CommandBufferLevel::Primary) allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		if (vkAllocateCommandBuffers(seDevice.getDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}
	SECommandBuffer::~SECommandBuffer() {
		vkFreeCommandBuffers(
			seDevice.getDevice(),
			commandPool,
			1,
			&commandBuffer
		);
	}
	void SECommandBuffer::begin(VkCommandBufferUsageFlags flags) {
		VkCommandBufferBeginInfo beginInfo{};
		//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT; // useful for renderpass only commands
		beginInfo.flags = flags;
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		
		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
	}
	void SECommandBuffer::end() {
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
	void SECommandBuffer::reset(VkCommandBufferResetFlags flags) {
		if (vkResetCommandBuffer(commandBuffer, flags) != VK_SUCCESS) {
			throw std::runtime_error("failed to reset command buffer!");
		}
	}
}