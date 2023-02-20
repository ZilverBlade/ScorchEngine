#pragma once

#include "texture2d.h"

namespace ScorchEngine {
	SETexture2D::SETexture2D(SEDevice& device, const SETexture::Builder& builder) : SETexture(device) {
		createTextureImage(builder);
		createTextureImageView(VK_IMAGE_VIEW_TYPE_2D);
		createTextureSampler();
		updateDescriptor();
	}
	SETexture2D::~SETexture2D()
	{
	}
	void SETexture2D::createTextureImage(const SETexture::Builder& builder) {
		mipLevels = 1;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		seDevice.createBuffer(
			builder.dataSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		void* data;
		vkMapMemory(seDevice.getDevice(), stagingBufferMemory, 0, builder.dataSize, 0, &data);
		memcpy(data, builder.pixels, builder.dataSize);
		vkUnmapMemory(seDevice.getDevice(), stagingBufferMemory);

		format = builder.srgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
		extent = { static_cast<uint32_t>(builder.width), static_cast<uint32_t>(builder.height), 1 };

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent = extent;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = layerCount;
		imageInfo.format = format;
		imageInfo.flags = 0;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		seDevice.createImageWithInfo(
			imageInfo,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			image,
			imageMemory
		);

		VkCommandBuffer commandBuffer = seDevice.beginSingleTimeCommands();

		seDevice.transitionImageLayout(
			commandBuffer,
			image,
			format,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			mipLevels,
			layerCount);
		seDevice.copyBufferToImage(
			commandBuffer,
			stagingBuffer,
			image,
			static_cast<uint32_t>(builder.width),
			static_cast<uint32_t>(builder.height),
			layerCount
		);
		seDevice.transitionImageLayout(
			commandBuffer,
			image,
			format,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			mipLevels,
			layerCount
		);
		layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		seDevice.endSingleTimeCommands(commandBuffer);

		vkDestroyBuffer(seDevice.getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(seDevice.getDevice(), stagingBufferMemory, nullptr);
	}
}