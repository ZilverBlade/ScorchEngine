#include "renderer.h"
#include <string>
#include <vulkan/vk_enum_string_helper.h>

namespace ScorchEngine {
	SERenderer::SERenderer(SEDevice& device) : seDevice(device), commandPool(device.getCommandPool()) {
		seQueue = seDevice.getAvailableQueue(SEQueueType::Graphics);
	}
	SERenderer::~SERenderer() {
		seDevice.freeAvailableQueue(seQueue);
	}
	void SERenderer::submitCommandBuffers(const std::vector<SECommandBuffer*>& commandBuffers) {
		for (SECommandBuffer* cb : commandBuffers) {
			toSubmitCommandBuffers.push_back(cb->getCommandBuffer());
		}
	}
	void SERenderer::submitQueue(VkSubmitInfo submitInfo, VkFence fence) {
		submitInfo.commandBufferCount = toSubmitCommandBuffers.size();
		submitInfo.pCommandBuffers = toSubmitCommandBuffers.data();
		if (VkResult result = vkQueueSubmit(seQueue->queue, 1, &submitInfo, fence); result != VK_SUCCESS) {
			throw std::runtime_error(std::string("failed to submit queue! Error code: " ) + string_VkResult(result));
		}
		toSubmitCommandBuffers.clear();
	}
}