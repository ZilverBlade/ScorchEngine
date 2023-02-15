#include "frame_buffer.h"
#include <scorch/vkapi/render_pass.h>

namespace ScorchEngine{

	SEFrameBuffer::SEFrameBuffer(SEDevice& device, SERenderPass* renderPass, const std::vector<SEFrameBufferAttachment*>& _attachments) : seDevice(device) {
		create(device, renderPass->getRenderPass(), _attachments);
		this->attachments.reserve(_attachments.size());
		for (SEFrameBufferAttachment* attachment : _attachments) {
			this->attachments.push_back(attachment);
		}
	}

	SEFrameBuffer::~SEFrameBuffer() {
		destroy();
	}

	void SEFrameBuffer::create(SEDevice& device, VkRenderPass renderPass, const std::vector<SEFrameBufferAttachment*>& newAttachments) {
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

	void SEFrameBuffer::destroy() {
		vkDestroyFramebuffer(seDevice.getDevice(), frameBuffer, nullptr);
	}

	void SEFrameBuffer::resize(glm::ivec3 newDimensions, SERenderPass* renderPass) {
		for (SEFrameBufferAttachment* attachment : attachments) {
			attachment->resize(newDimensions);
		}
		destroy();
		create(seDevice, renderPass->getRenderPass(), attachments);
	}

}