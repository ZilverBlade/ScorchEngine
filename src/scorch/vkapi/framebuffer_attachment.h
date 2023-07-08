#pragma once

#include <scorch/vkapi/device.h>
#include <glm/glm.hpp>

namespace ScorchEngine {
	enum class SEFramebufferAttachmentType {
		Depth,
		Color,
		Resolve
	};

	struct SEFramebufferAttachmentCreateInfo {
		VkFormat framebufferFormat;
		VkImageAspectFlags imageAspect;
		VkImageViewType viewType;
		glm::ivec3 dimensions;
		VkImageUsageFlags usage;
		VkImageLayout layout;
		SEFramebufferAttachmentType framebufferType;
		bool linearFiltering = true;
		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
		bool isShadowMap = false;
		uint32_t mipLevels = 1;
		uint32_t layerCount = 1;
	};

	class SEFramebufferAttachment {
	public:
		SEFramebufferAttachment(SEDevice& device, const SEFramebufferAttachmentCreateInfo& attachmentCreateInfo);
		~SEFramebufferAttachment();

		SEFramebufferAttachment(const SEFramebufferAttachment&) = delete;
		SEFramebufferAttachment& operator=(const SEFramebufferAttachment&) = delete;
		SEFramebufferAttachment(SEFramebufferAttachment&&) = delete;
		SEFramebufferAttachment& operator=(SEFramebufferAttachment&&) = delete;

		VkImage getImage() { return image; }
		VkImageSubresourceRange getImageSubresourceRange() { return subresourceRange; }
		VkImageView getImageView() { return imageView; }
		VkImageView getSubImageView(uint32_t layer, uint32_t mipLevel) {
			if (subImageViews.empty()) { // if layerCount == 1 and mipLevels == 1, there is no need for special image views for renderpasses
				return imageView;
			}
			return subImageViews[layer][mipLevel];
		}
		VkSampler getSampler() { return sampler; }
		VkImageLayout getImageLayout() { return attachmentDescription.layout; }
		VkDeviceMemory getDeviceMemory() { return imageMemory; }
		glm::ivec3 getDimensions() { return dimensions; }
		VkDescriptorImageInfo getDescriptor() {
			return VkDescriptorImageInfo{
				sampler,
				imageView,
				attachmentDescription.layout
			};
		}
		SEFramebufferAttachmentType getAttachmentType() { return attachmentDescription.framebufferType; }
		const SEFramebufferAttachmentCreateInfo& getAttachmentDescription() { return attachmentDescription; }

		void resize(glm::ivec3 newDimensions);
	private:
		void destroy();
		void create(SEDevice& device, const SEFramebufferAttachmentCreateInfo& attachmentCreateInfo);

		VkImage image{};
		VkDeviceMemory imageMemory{};
		VkSampler sampler{};
		VkImageSubresourceRange subresourceRange{};
		VkImageView imageView{};

		std::vector<std::vector<VkImageView>> subImageViews{};
		std::vector<std::vector<VkImageSubresourceRange>> subSubresourceRanges{};

		SEFramebufferAttachmentCreateInfo attachmentDescription;

		SEDevice& seDevice;

		glm::ivec3 dimensions{};
	};
}