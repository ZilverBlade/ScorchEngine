#pragma once

#include <scorch/vkapi/device.h>
#include <glm/glm.hpp>

namespace ScorchEngine {
	enum class FrameBufferAttachmentType {
		Depth,
		Color,
		Resolve
	};

	struct FrameBufferAttachmentCreateInfo {
		VkFormat frameBufferFormat;
		VkImageAspectFlags imageAspect;
		glm::ivec3 dimensions;
		VkImageUsageFlags usage;
		VkImageLayout layout;
		FrameBufferAttachmentType frameBufferType;
		bool linearFiltering = true;
		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
	};

	class FrameBufferAttachment {
	public:
		FrameBufferAttachment(SEDevice& device, const FrameBufferAttachmentCreateInfo& attachmentCreateInfo);
		~FrameBufferAttachment();

		FrameBufferAttachment(const FrameBufferAttachment&) = delete;
		FrameBufferAttachment& operator=(const FrameBufferAttachment&) = delete;
		FrameBufferAttachment(FrameBufferAttachment&&) = delete;
		FrameBufferAttachment& operator=(FrameBufferAttachment&&) = delete;

		VkImage getImage() { return image; }
		VkImageSubresourceRange getImageSubresourceRange() { return subresourceRange; }
		VkImageView getImageView() { return imageView; }
		VkSampler getSampler() { return sampler; }
		VkImageLayout getImageLayout() { return attachmentDescription.layout; }
		VkDeviceMemory getDeviceMemory() { return imageMemory; }
		glm::ivec3 getDimensions() { return dimensions; }
		VkDescriptorImageInfo& getDescriptor() { return descriptor; }
		FrameBufferAttachmentType getAttachmentType() { return attachmentDescription.frameBufferType; }
		const FrameBufferAttachmentCreateInfo& getAttachmentDescription() { return attachmentDescription; }

		void resize(glm::ivec3 newDimensions);
	private:
		void destroy();
		void create(SEDevice& device, const FrameBufferAttachmentCreateInfo& attachmentCreateInfo);

		VkImage image{};
		VkImageSubresourceRange subresourceRange{};
		VkImageView imageView{};
		VkSampler sampler{};
		VkDeviceMemory imageMemory{};
		VkDescriptorImageInfo descriptor{};

		FrameBufferAttachmentCreateInfo attachmentDescription;

		SEDevice& seDevice;

		glm::ivec3 dimensions{};
	};
}