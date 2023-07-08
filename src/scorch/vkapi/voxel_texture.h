#pragma once

#include <scorch/vkapi/device.h>
#include <glm/glm.hpp>

namespace ScorchEngine {
	struct SEVoxelTextureCreateInfo {
		VkFormat voxelFormat;
		glm::ivec3 dimensions;
		VkImageUsageFlags usage;
		VkImageLayout layout;
		bool linearFiltering = true;
		uint32_t mipLevels = 1;
	};

	class SEVoxelTexture {
	public:
		SEVoxelTexture(SEDevice& device, const SEVoxelTextureCreateInfo& createInfo);
		~SEVoxelTexture();

		SEVoxelTexture(const SEVoxelTexture&) = delete;
		SEVoxelTexture& operator=(const SEVoxelTexture&) = delete;
		SEVoxelTexture(SEVoxelTexture&&) = delete;
		SEVoxelTexture& operator=(SEVoxelTexture&&) = delete;

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
		void create(SEDevice& device, const SEVoxelTextureCreateInfo& createInfo);

		VkImage image{};
		VkDeviceMemory imageMemory{};
		VkSampler sampler{};
		VkImageSubresourceRange subresourceRange{};
		VkImageView imageView{};

		VkFormat voxelFormat;
		glm::ivec3 dimensions;
		VkImageUsageFlags imageUsage;
		VkImageLayout imageLayout;

		SEDevice& seDevice;
	};
}