#pragma once
#include <scorch/vkapi/device.h>
namespace ScorchEngine {
	enum class CommandBufferLevel {
		Primary,
		Secondary
	};
	class SECommandBuffer {
	public:
		SECommandBuffer(SEDevice& device, CommandBufferLevel level = CommandBufferLevel::Primary);
		~SECommandBuffer();
		SECommandBuffer(const SECommandBuffer&) = delete;
		SECommandBuffer& operator=(const SECommandBuffer&) = delete;

		VkCommandBuffer getCommandBuffer() {
			return commandBuffer;
		}
		void begin(VkCommandBufferUsageFlags flags = VkFlags());
		void end();
		void reset(VkCommandBufferResetFlags flags = VkFlags());
	private:
		VkCommandBuffer commandBuffer{};
		VkCommandPool commandPool;
		SEDevice& seDevice;
	};

}