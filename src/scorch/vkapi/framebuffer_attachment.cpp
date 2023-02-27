#include "framebuffer_attachment.h"
namespace ScorchEngine {
	SEFramebufferAttachment::SEFramebufferAttachment(SEDevice& device, const SEFramebufferAttachmentCreateInfo& attachmentCreateInfo) : seDevice(device), attachmentDescription(attachmentCreateInfo) {
		create(device, attachmentCreateInfo);
	}
	SEFramebufferAttachment::~SEFramebufferAttachment() {
		destroy();
	}

	void SEFramebufferAttachment::create(SEDevice& device, const SEFramebufferAttachmentCreateInfo& attachmentCreateInfo) {
		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = attachmentCreateInfo.framebufferFormat;
		imageCreateInfo.extent.width = attachmentCreateInfo.dimensions.x;
		imageCreateInfo.extent.height = attachmentCreateInfo.dimensions.y;
		imageCreateInfo.extent.depth = attachmentCreateInfo.dimensions.z;
		imageCreateInfo.mipLevels = attachmentCreateInfo.mipLevels;
		imageCreateInfo.arrayLayers = attachmentCreateInfo.layerCount;
		imageCreateInfo.samples = attachmentCreateInfo.sampleCount;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = attachmentCreateInfo.usage;
		VkMemoryAllocateInfo memAlloc = {};
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		VkMemoryRequirements memReqs;

		if (vkCreateImage(device.getDevice(), &imageCreateInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image!");
		}
		vkGetImageMemoryRequirements(device.getDevice(), image, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device.getDevice(), &memAlloc, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate memory!");
		}

		if (vkBindImageMemory(device.getDevice(), image, imageMemory, 0) != VK_SUCCESS) {
			throw std::runtime_error("Failed to bind memory!");
		}
		
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = attachmentCreateInfo.viewType;
		imageViewCreateInfo.format = attachmentCreateInfo.framebufferFormat;
		subresourceRange.aspectMask = attachmentCreateInfo.imageAspect;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = attachmentCreateInfo.mipLevels;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = attachmentCreateInfo.layerCount;
		imageViewCreateInfo.subresourceRange = subresourceRange;
		imageViewCreateInfo.image = image;

		if (vkCreateImageView(device.getDevice(), &imageViewCreateInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image view!");
		}
		if (attachmentCreateInfo.layerCount > 1 || attachmentCreateInfo.mipLevels > 1) {
			subImageViews.resize(attachmentCreateInfo.layerCount);
			subSubresourceRanges.resize(attachmentCreateInfo.layerCount);
			for (int i = 0; i < attachmentCreateInfo.layerCount; i++) {
				subImageViews[i].resize(attachmentCreateInfo.mipLevels);
				subSubresourceRanges[i].resize(attachmentCreateInfo.mipLevels);
				for (int j = 0; j < attachmentCreateInfo.mipLevels; j++) {
					VkImageViewCreateInfo mipImageViewCreateInfo = {};
					mipImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
					mipImageViewCreateInfo.viewType = attachmentCreateInfo.viewType;
					mipImageViewCreateInfo.format = attachmentCreateInfo.framebufferFormat;
					subSubresourceRanges[i][j].aspectMask = attachmentCreateInfo.imageAspect;
					subSubresourceRanges[i][j].baseMipLevel = j;
					subSubresourceRanges[i][j].levelCount = 1;
					subSubresourceRanges[i][j].baseArrayLayer = i;
					subSubresourceRanges[i][j].layerCount = 1;
					mipImageViewCreateInfo.subresourceRange = subSubresourceRanges[i][j];
					mipImageViewCreateInfo.image = image;

					if (vkCreateImageView(device.getDevice(), &imageViewCreateInfo, nullptr, &subImageViews[i][j]) != VK_SUCCESS) {
						throw std::runtime_error("Failed to create image view!");
					}
				}
			}
		}	

		if (attachmentCreateInfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
			// Create sampler to sample from the attachment in the fragment shader
			VkSamplerCreateInfo samplerInfo = {};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = (attachmentCreateInfo.linearFiltering) ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
			samplerInfo.minFilter = (attachmentCreateInfo.linearFiltering) ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = (attachmentCreateInfo.linearFiltering) ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.maxAnisotropy = 1.0f;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = static_cast<float>(attachmentCreateInfo.mipLevels);
			samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			samplerInfo.addressModeV = samplerInfo.addressModeU;
			samplerInfo.addressModeW = samplerInfo.addressModeU;
			if (vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create sampler");
			}
		}
		dimensions = { attachmentCreateInfo.dimensions.x, attachmentCreateInfo.dimensions.y, attachmentCreateInfo.dimensions.z };
	}
	void SEFramebufferAttachment::destroy() {
		vkDestroyImageView(seDevice.getDevice(), imageView, nullptr);
		if (attachmentDescription.layerCount > 1 || attachmentDescription.mipLevels > 1) {
			for (int i = 0; i < attachmentDescription.layerCount; i++) {
				for (int j = 0; j < attachmentDescription.mipLevels; j++) {
					vkDestroyImageView(seDevice.getDevice(), subImageViews[i][j], nullptr);
				}
			}
		}	
		vkDestroySampler(seDevice.getDevice(), sampler, nullptr);
		vkDestroyImage(seDevice.getDevice(), image, nullptr);
		vkFreeMemory(seDevice.getDevice(), imageMemory, nullptr);
	}

	void SEFramebufferAttachment::resize(glm::ivec3 newDimensions) {
		if (newDimensions == attachmentDescription.dimensions) return; // in case already been resized
		destroy();
		attachmentDescription.dimensions = newDimensions;
		create(seDevice, attachmentDescription);
	}	
}
