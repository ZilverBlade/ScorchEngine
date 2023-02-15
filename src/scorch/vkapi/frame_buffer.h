#pragma once

#include <scorch/vkapi/frame_buffer_attachment.h>

namespace ScorchEngine {
	class FrameBuffer {
	public:
		FrameBuffer(SEDevice& device, VkRenderPass renderPass, const std::vector<FrameBufferAttachment*>& attachments);
		~FrameBuffer();

		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer& operator=(const FrameBuffer&) = delete;
		FrameBuffer(FrameBuffer&&) = delete;
		FrameBuffer& operator=(FrameBuffer&&) = delete;

		inline glm::ivec3 getDimensions() { return { width, height, depth }; }
		inline VkFramebuffer getFrameBuffer() { return frameBuffer; }
		void resize(glm::ivec3 newDimensions, VkRenderPass renderpass);
	private:
		void create(SEDevice& device, VkRenderPass renderPass, const std::vector<FrameBufferAttachment*>& _attachments);
		void destroy();

		uint32_t width{};
		uint32_t height{};
		uint32_t depth{};

		VkFramebuffer frameBuffer{};

		std::vector<FrameBufferAttachment*> attachments;
		SEDevice& seDevice;
	};
}