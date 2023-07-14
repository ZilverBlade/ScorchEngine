#include "empty_texture.h"
namespace ScorchEngine {
	SEEmptyTexture::SEEmptyTexture(SEDevice& device, const SEEmptyTextureCreateInfo& createInfo)
		: seDevice(device), format(createInfo.format), imageUsage(createInfo.usage), imageLayout(createInfo.layout) {
		create(device, createInfo);
	}
	SEEmptyTexture::~SEEmptyTexture() {
		destroy();
	}

	void SEEmptyTexture::create(SEDevice& device, const SEEmptyTextureCreateInfo& createInfo) {
		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = createInfo.imageType;
		imageCreateInfo.format = createInfo.format;
		imageCreateInfo.extent.width = createInfo.dimensions.x;
		imageCreateInfo.extent.height = createInfo.dimensions.y;
		imageCreateInfo.extent.depth = createInfo.dimensions.z;
		imageCreateInfo.mipLevels = createInfo.mipLevels;
		imageCreateInfo.arrayLayers = createInfo.layers;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = createInfo.usage;
		//imageCreateInfo.
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
		imageViewCreateInfo.viewType = createInfo.viewType;
		imageViewCreateInfo.format = createInfo.format;
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = createInfo.mipLevels;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange = subresourceRange;
		imageViewCreateInfo.image = image;

		if (vkCreateImageView(device.getDevice(), &imageViewCreateInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image view!");
		}
		if (createInfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
			// Create sampler to sample from the attachment in the fragment shader
			VkSamplerCreateInfo samplerInfo = {};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = createInfo.linearFiltering ? VK_FILTER_LINEAR :  VK_FILTER_NEAREST;
			samplerInfo.minFilter = createInfo.linearFiltering ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
			samplerInfo.mipmapMode = createInfo.linearFiltering ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.maxAnisotropy = 1.0f;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = static_cast<float>(createInfo.mipLevels);
			samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			samplerInfo.addressModeV = samplerInfo.addressModeU;
			samplerInfo.addressModeW = samplerInfo.addressModeU;
			samplerInfo.compareEnable = VK_FALSE;

			if (vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create sampler");
			}
		}
		dimensions = { createInfo.dimensions.x, createInfo.dimensions.y, createInfo.dimensions.z };
	}
	void SEEmptyTexture::destroy() {
		vkDestroyImageView(seDevice.getDevice(), imageView, nullptr);
		vkDestroySampler(seDevice.getDevice(), sampler, nullptr);
		vkDestroyImage(seDevice.getDevice(), image, nullptr);
		vkFreeMemory(seDevice.getDevice(), imageMemory, nullptr);
	}
}
