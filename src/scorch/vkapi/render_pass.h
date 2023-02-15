#pragma once

#include <scorch/vkapi/frame_buffer.h>

namespace ScorchEngine {
	struct AttachmentClearColor {
		VkClearColorValue color{{0.f, 0.f, 0.f, 1.f}};
		VkClearDepthStencilValue depth{ 1.f, 0 };
	};
	struct AttachmentInfo {
		FrameBufferAttachment* frameBufferAttachment;
		VkAttachmentLoadOp loadOp;
		VkAttachmentStoreOp storeOp;
		AttachmentClearColor clear = AttachmentClearColor{};
	};

	class RenderPass {
	public:
		RenderPass(SEDevice& device, const std::vector<AttachmentInfo>& attachments);
		~RenderPass();

		RenderPass(const RenderPass&) = delete;
		RenderPass& operator=(const RenderPass&) = delete;
		RenderPass(RenderPass&&) = delete;
		RenderPass& operator=(RenderPass&&) = delete;

		void beginRenderPass(VkCommandBuffer commandbuffer, FrameBuffer* frameBuffer);
		void endRenderPass(VkCommandBuffer commandbuffer);

		inline void setViewportSize(uint32_t _width, uint32_t _height) { width = _width; height = _height; }

		inline VkRenderPass getRenderPass() { return renderpass; }
	private:
		uint32_t width{};
		uint32_t height{};

		SEDevice& seDevice;

		std::vector<VkClearValue> clearValues{};
		VkRenderPass renderpass{};
	};
}