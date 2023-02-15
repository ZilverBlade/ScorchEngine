#pragma once

#include <scorch/vkapi/frame_buffer_attachment.h>

namespace ScorchEngine {
	class SERenderPass;
	class SEFrameBuffer {
	public:
		SEFrameBuffer(SEDevice& device, SERenderPass* renderPass, const std::vector<SEFrameBufferAttachment*>& attachments);
		~SEFrameBuffer();

		SEFrameBuffer(const SEFrameBuffer&) = delete;
		SEFrameBuffer& operator=(const SEFrameBuffer&) = delete;
		SEFrameBuffer(SEFrameBuffer&&) = delete;
		SEFrameBuffer& operator=(SEFrameBuffer&&) = delete;

		inline glm::ivec3 getDimensions() { return { width, height, depth }; }
		inline VkFramebuffer getFrameBuffer() { return frameBuffer; }
		void resize(glm::ivec3 newDimensions, SERenderPass* renderPass);
	private:
		void create(SEDevice& device, VkRenderPass renderPass, const std::vector<SEFrameBufferAttachment*>& _attachments);
		void destroy();

		uint32_t width{};
		uint32_t height{};
		uint32_t depth{};

		VkFramebuffer frameBuffer{};

		std::vector<SEFrameBufferAttachment*> attachments;
		SEDevice& seDevice;
	};
}