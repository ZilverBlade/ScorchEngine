#include "framebuffer.h"
#include <scorch/vkapi/render_pass.h>

namespace ScorchEngine{

	SEFramebuffer::SEFramebuffer(SEDevice& device, SERenderPass* renderPass, const std::vector<SEFramebufferAttachment*>& attachments, uint32_t layer, uint32_t mipLevel) : seDevice(device), useLayer(layer), useMipLevel(mipLevel) {
		create(device, renderPass->getRenderPass(), attachments);
		this->attachments.reserve(attachments.size());
		for (SEFramebufferAttachment* attachment : attachments) {
			this->attachments.push_back(attachment);
		}
	}

	SEFramebuffer::~SEFramebuffer() {
		destroy();
	}

	void SEFramebuffer::create(SEDevice& device, VkRenderPass renderPass, const std::vector<SEFramebufferAttachment*>& newAttachments) {
		std::vector<VkImageView> imageViews;
		for (auto& attachment : newAttachments) {
			imageViews.push_back(attachment->getSubImageView(useLayer, useMipLevel));
		}

		width = static_cast<uint32_t>(newAttachments[0]->getDimensions().x) / std::pow(2, useMipLevel);
		height = static_cast<uint32_t>(newAttachments[0]->getDimensions().y) / std::pow(2, useMipLevel);
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
		if (vkCreateFramebuffer(device.getDevice(), &fbufferCreateInfo, nullptr, &framebuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}

	void SEFramebuffer::destroy() {
		vkDestroyFramebuffer(seDevice.getDevice(), framebuffer, nullptr);
	}

	void SEFramebuffer::resize(glm::ivec3 newDimensions, SERenderPass* renderPass) {
		for (SEFramebufferAttachment* attachment : attachments) {
			attachment->resize(newDimensions);
		}
		destroy();
		create(seDevice, renderPass->getRenderPass(), attachments);
	}

}