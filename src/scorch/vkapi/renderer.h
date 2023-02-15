#pragma once

#include <scorch/vkapi/device.h>
#include <scorch/vkapi/command_buffer.h>

namespace ScorchEngine {
	class SERenderer {
	public:
		SERenderer(SEDevice& device);
		~SERenderer();

		SERenderer(const SERenderer&) = delete;
		SERenderer& operator=(const SERenderer&) = delete;
		SERenderer(SERenderer&&) = delete;
		SERenderer& operator=(SERenderer&&) = delete;

		void submitCommandBuffers(const std::vector<SECommandBuffer*>& commandBuffers);
		void submitQueue(VkSubmitInfo submitInfo, VkFence fence);
		VkQueue getQueue() {
			return seQueue->queue;
		}
	private:
		std::vector<VkCommandBuffer> toSubmitCommandBuffers{};
		VkCommandPool commandPool;
		SEDevice& seDevice;
		SEQueue* seQueue;
	};
}