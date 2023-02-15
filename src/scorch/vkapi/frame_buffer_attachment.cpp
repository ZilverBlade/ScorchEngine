#include "frame_buffer_attachment.h"
namespace ScorchEngine {
	SEFrameBufferAttachment::SEFrameBufferAttachment(SEDevice& device, const SEFrameBufferAttachmentCreateInfo& attachmentCreateInfo) : seDevice(device), attachmentDescription(attachmentCreateInfo) {
		create(device, attachmentCreateInfo);
	}
	SEFrameBufferAttachment::~SEFrameBufferAttachment() {
		destroy();
	}

	void SEFrameBufferAttachment::create(SEDevice& device, const SEFrameBufferAttachmentCreateInfo& attachmentCreateInfo) {
		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = attachmentCreateInfo.frameBufferFormat;
		imageCreateInfo.extent.width = attachmentCreateInfo.dimensions.x;
		imageCreateInfo.extent.height = attachmentCreateInfo.dimensions.y;
		imageCreateInfo.extent.depth = attachmentCreateInfo.dimensions.z;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
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
		imageViewCreateInfo.viewType =  VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = attachmentCreateInfo.frameBufferFormat;
		subresourceRange.aspectMask = attachmentCreateInfo.imageAspect;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange = subresourceRange;
		imageViewCreateInfo.image = image;


		if (vkCreateImageView(device.getDevice(), &imageViewCreateInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image view!");
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
			samplerInfo.maxLod = 1.0f;
			samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			samplerInfo.addressModeV = samplerInfo.addressModeU;
			samplerInfo.addressModeW = samplerInfo.addressModeU;
			if (vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create sampler");
			}

			descriptor.sampler = sampler;
		}
		// Fill a descriptor for later use in a descriptor set
		descriptor.imageLayout = attachmentCreateInfo.layout;
		descriptor.imageView = imageView;
		dimensions = { attachmentCreateInfo.dimensions.x, attachmentCreateInfo.dimensions.y, attachmentCreateInfo.dimensions.z };
	}
	void SEFrameBufferAttachment::destroy() {
		vkDestroyImageView(seDevice.getDevice(), imageView, nullptr);
		vkDestroyImage(seDevice.getDevice(), image, nullptr);
		vkDestroySampler(seDevice.getDevice(), sampler, nullptr);
		vkFreeMemory(seDevice.getDevice(), imageMemory, nullptr);
	}

	void SEFrameBufferAttachment::resize(glm::ivec3 newDimensions) {
		if (newDimensions == attachmentDescription.dimensions) return; // in case already been resized
		destroy();
		attachmentDescription.dimensions = newDimensions;
		create(seDevice, attachmentDescription);
	}	
}
