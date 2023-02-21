#pragma once

#define NOMINMAX
#include "texture_cube.h"

namespace ScorchEngine {
	SETextureCube::SETextureCube(SEDevice& device, const SETexture::Builder& builder) : SETexture(device) {
		anisotropy = 1.0;
		layerCount = 6;
		filter = VK_FILTER_LINEAR;
		createTextureImage(builder);
		createTextureImageView(VK_IMAGE_VIEW_TYPE_CUBE);
		createTextureSampler();
		updateDescriptor();
	}
	SETextureCube::~SETextureCube()
	{
	}
	void SETextureCube::createTextureImage(const SETexture::Builder& builder) {
		mipLevels = std::floor(std::log2(std::max(builder.height, builder.width))) + 1;

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
		size_t layerSize = builder.dataSize / 6;
		for (int i = 0; i < 6; i++) {
			memcpy(static_cast<char*>(data) + layerSize * i, builder.pixels[i], layerSize);
		}
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
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
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
		seDevice.generateMipMaps(
			commandBuffer,
			image,
			format,
			static_cast<uint32_t>(builder.width),
			static_cast<uint32_t>(builder.height),
			mipLevels,
			layerCount,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
		layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		seDevice.endSingleTimeCommands(commandBuffer);

		vkDestroyBuffer(seDevice.getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(seDevice.getDevice(), stagingBufferMemory, nullptr);
	}
}