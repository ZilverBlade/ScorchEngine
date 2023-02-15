#include "frame_buffer.h"

namespace ScorchEngine{

	FrameBuffer::FrameBuffer(SEDevice& device, VkRenderPass renderPass, const std::vector<FrameBufferAttachment*>& _attachments) : seDevice(device) {
		create(device, renderPass, _attachments);
		this->attachments.reserve(_attachments.size());
		for (FrameBufferAttachment* attachment : _attachments) {
			this->attachments.push_back(attachment);
		}
	}

	FrameBuffer::~FrameBuffer() {
		destroy();
	}

	void FrameBuffer::create(SEDevice& device, VkRenderPass renderPass, const std::vector<FrameBufferAttachment*>& newAttachments) {
		std::vector<VkImageView> imageViews;
		for (auto& attachment : newAttachments) {
			imageViews.push_back(attachment->getImageView());
		}

		width = static_cast<uint32_t>(newAttachments[0]->getDimensions().x);
		height = static_cast<uint32_t>(newAttachments[0]->getDimensions().y);
		depth = static_cast<uint32_t>(newAttachments[0]->getDimensions().z);

		VkFramebufferCreateInfo fbufferCreateInfo{};
		fbufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbufferCreateInfo.pNext = nullptr;
		fbufferCreateInfo.flags = 0;
		fbufferCreateInfo.renderPass = renderPass;
		fbufferCreateInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
		fbufferCreateInfo.pAttachments = imageViews.data();
		fbufferCreateInfo.width = width;
		fbufferCreateInfo.height = height;
		fbufferCreateInfo.layers = 1;
		if (vkCreateFramebuffer(device.getDevice(), &fbufferCreateInfo, nullptr, &frameBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}

	void FrameBuffer::destroy() {
		vkDestroyFramebuffer(seDevice.getDevice(), frameBuffer, nullptr);
	}

	void FrameBuffer::resize(glm::ivec3 newDimensions, VkRenderPass renderpass) {
		for (FrameBufferAttachment* attachment : attachments) {
			attachment->resize(newDimensions);
		}
		destroy();
		create(seDevice, renderpass, attachments);
	}

}