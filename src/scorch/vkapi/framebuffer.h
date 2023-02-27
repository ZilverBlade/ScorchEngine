#pragma once

#include <scorch/vkapi/framebuffer_attachment.h>

namespace ScorchEngine {
	class SERenderPass;
	class SEFramebuffer {
	public:
		SEFramebuffer(SEDevice& device, SERenderPass* renderPass, const std::vector<SEFramebufferAttachment*>& attachments, uint32_t layer = 0, uint32_t mipLevel = 0);
		~SEFramebuffer();

		SEFramebuffer(const SEFramebuffer&) = delete;
		SEFramebuffer& operator=(const SEFramebuffer&) = delete;
		SEFramebuffer(SEFramebuffer&&) = delete;
		SEFramebuffer& operator=(SEFramebuffer&&) = delete;

		inline glm::ivec3 getDimensions() { return { width, height, depth }; }
		inline VkFramebuffer getFramebuffer() { return framebuffer; }
		void resize(glm::ivec3 newDimensions, SERenderPass* renderPass);
	private:
		void create(SEDevice& device, VkRenderPass renderPass, const std::vector<SEFramebufferAttachment*>& attachments);
		void destroy();

		uint32_t width{};
		uint32_t height{};
		uint32_t depth{};

		VkFramebuffer framebuffer{};
		uint32_t useMipLevel = 0;
		uint32_t useLayer = 0;

		std::vector<SEFramebufferAttachment*> attachments;
		SEDevice& seDevice;
	};
}