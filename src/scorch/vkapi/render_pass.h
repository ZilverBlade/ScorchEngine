#pragma once

#include <scorch/vkapi/frame_buffer.h>

namespace ScorchEngine {
	struct SEAttachmentClearColor {
		VkClearColorValue color{{0.f, 0.f, 0.f, 1.f}};
		VkClearDepthStencilValue depth{ 1.f, 0 };
	};
	struct SEAttachmentInfo {
		SEFrameBufferAttachment* frameBufferAttachment;
		VkAttachmentLoadOp loadOp;
		VkAttachmentStoreOp storeOp;
		SEAttachmentClearColor clear = SEAttachmentClearColor{};
	};

	class SERenderPass {
	public:
		SERenderPass(SEDevice& device, const std::vector<SEAttachmentInfo>& attachments);
		~SERenderPass();

		SERenderPass(const SERenderPass&) = delete;
		SERenderPass& operator=(const SERenderPass&) = delete;
		SERenderPass(SERenderPass&&) = delete;
		SERenderPass& operator=(SERenderPass&&) = delete;

		void beginRenderPass(VkCommandBuffer commandbuffer, SEFrameBuffer* frameBuffer);
		void endRenderPass(VkCommandBuffer commandbuffer);

		void setViewportSize(uint32_t w, uint32_t h) { width = w; height = h; }

		VkRenderPass getRenderPass() { return renderpass; }
	private:
		uint32_t width{};
		uint32_t height{};

		SEDevice& seDevice;

		std::vector<VkClearValue> clearValues{};
		VkRenderPass renderpass{};
	};
}