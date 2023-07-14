#pragma once

#include <scorch/vkapi/device.h>
#include <glm/glm.hpp>

namespace ScorchEngine {
	struct SEEmptyTextureCreateInfo {
		VkFormat format;
		VkImageType imageType;
		VkImageViewType viewType;
		glm::ivec3 dimensions;
		VkImageUsageFlags usage;
		VkImageLayout layout;
		bool linearFiltering = true;
		uint32_t layers = 1;
		uint32_t mipLevels = 1;
	};

	class SEEmptyTexture {
	public:
		SEEmptyTexture(SEDevice& device, const SEEmptyTextureCreateInfo& createInfo);
		~SEEmptyTexture();

		SEEmptyTexture(const SEEmptyTexture&) = delete;
		SEEmptyTexture& operator=(const SEEmptyTexture&) = delete;
		SEEmptyTexture(SEEmptyTexture&&) = delete;
		SEEmptyTexture& operator=(SEEmptyTexture&&) = delete;

		VkImage getImage() { return image; }
		VkImageSubresourceRange getImageSubresourceRange() { return subresourceRange; }
		VkImageView getImageView() { return imageView; }
		VkSampler getSampler() { return sampler; }
		VkImageLayout getImageLayout() { return imageLayout; }
		VkDeviceMemory getDeviceMemory() { return imageMemory; }
		glm::ivec3 getDimensions() { return dimensions; }
		VkDescriptorImageInfo getDescriptor() {
			return VkDescriptorImageInfo{
				sampler,
				imageView,
				imageLayout
			};
		}
	private:
		void destroy();
		void create(SEDevice& device, const SEEmptyTextureCreateInfo& createInfo);

		VkImage image{};
		VkDeviceMemory imageMemory{};
		VkSampler sampler{};
		VkImageSubresourceRange subresourceRange{};
		VkImageView imageView{};

		VkFormat format;
		glm::ivec3 dimensions;
		VkImageUsageFlags imageUsage;
		VkImageLayout imageLayout;

		SEDevice& seDevice;
	};
}