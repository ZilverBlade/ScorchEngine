#pragma once

#include <scorch/vkapi/device.h>
#include <glm/glm.hpp>

namespace ScorchEngine {
	enum class SEFrameBufferAttachmentType {
		Depth,
		Color,
		Resolve
	};

	struct SEFrameBufferAttachmentCreateInfo {
		VkFormat frameBufferFormat;
		VkImageAspectFlags imageAspect;
		glm::ivec3 dimensions;
		VkImageUsageFlags usage;
		VkImageLayout layout;
		SEFrameBufferAttachmentType frameBufferType;
		bool linearFiltering = true;
		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
		VkImage swapchainImage = nullptr;
	};

	class SEFrameBufferAttachment {
	public:
		SEFrameBufferAttachment(SEDevice& device, const SEFrameBufferAttachmentCreateInfo& attachmentCreateInfo);
		~SEFrameBufferAttachment();

		SEFrameBufferAttachment(const SEFrameBufferAttachment&) = delete;
		SEFrameBufferAttachment& operator=(const SEFrameBufferAttachment&) = delete;
		SEFrameBufferAttachment(SEFrameBufferAttachment&&) = delete;
		SEFrameBufferAttachment& operator=(SEFrameBufferAttachment&&) = delete;

		VkImage getImage() { return image; }
		VkImageSubresourceRange getImageSubresourceRange() { return subresourceRange; }
		VkImageView getImageView() { return imageView; }
		VkSampler getSampler() { return sampler; }
		VkImageLayout getImageLayout() { return attachmentDescription.layout; }
		VkDeviceMemory getDeviceMemory() { return imageMemory; }
		glm::ivec3 getDimensions() { return dimensions; }
		VkDescriptorImageInfo getDescriptor() { return descriptor; }
		SEFrameBufferAttachmentType getAttachmentType() { return attachmentDescription.frameBufferType; }
		const SEFrameBufferAttachmentCreateInfo& getAttachmentDescription() { return attachmentDescription; }

		void resize(glm::ivec3 newDimensions);
	private:
		void destroy();
		void create(SEDevice& device, const SEFrameBufferAttachmentCreateInfo& attachmentCreateInfo);

		VkImage image{};
		VkImageSubresourceRange subresourceRange{};
		VkImageView imageView{};
		VkSampler sampler{};
		VkDeviceMemory imageMemory{};
		VkDescriptorImageInfo descriptor{};

		SEFrameBufferAttachmentCreateInfo attachmentDescription;

		SEDevice& seDevice;

		bool vkImageIsSwapChainImage = false;
		glm::ivec3 dimensions{};
	};
}