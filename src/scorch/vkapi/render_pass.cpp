#include "render_pass.h"
#include <array>

namespace ScorchEngine {
	SERenderPass::SERenderPass(SEDevice& device, const std::vector<SEAttachmentInfo>& attachments) : seDevice(device) {
		std::vector<VkAttachmentDescription> attachmentDescriptions;
		attachmentDescriptions.resize(attachments.size());

		int colorAttachmentLoadCount = 0;
		int depthAttachmentLoadCount = 0;

		for (int i = 0; i < attachmentDescriptions.size(); i++) {
			auto& attachmentDescription = attachmentDescriptions[i];
			if (attachments[i].framebufferAttachment->getAttachmentType() == SEFramebufferAttachmentType::Resolve) {
				assert(attachments[i].loadOp == VK_ATTACHMENT_LOAD_OP_DONT_CARE && "Resolve attachments should have VK_ATTACHMENT_LOAD_OP_DONT_CARE set as the loadOp!");
			}
			attachmentDescription.format = attachments[i].framebufferAttachment->getAttachmentDescription().framebufferFormat;
			attachmentDescription.samples = attachments[i].framebufferAttachment->getAttachmentDescription().sampleCount;
			attachmentDescription.loadOp = attachments[i].loadOp;
			attachmentDescription.storeOp = attachments[i].storeOp;
			attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDescription.initialLayout = attachmentDescription.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD ? attachments[i].framebufferAttachment->getImageLayout() : VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescription.finalLayout = attachments[i].framebufferAttachment->getAttachmentDescription().layout;

			if ((attachmentDescription.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) && (attachments[i].framebufferAttachment->getAttachmentType() == SEFramebufferAttachmentType::Color))
				colorAttachmentLoadCount ++;
			if ((attachmentDescription.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) && (attachments[i].framebufferAttachment->getAttachmentType() == SEFramebufferAttachmentType::Depth))
				depthAttachmentLoadCount ++;
		}

		std::vector<VkAttachmentReference> colorAttachments{};
		std::vector<VkAttachmentReference> depthAttachments{};
		std::vector<VkAttachmentReference> resolveAttachments{};
		VkSubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.pColorAttachments = nullptr;
		subpassDescription.pDepthStencilAttachment = nullptr;
		subpassDescription.pResolveAttachments = nullptr;
		
		for (uint32_t i = 0; i < attachments.size(); i++) {
			VkAttachmentReference attachmentRef{};
			switch(attachments[i].framebufferAttachment->getAttachmentType()) {
			case(SEFramebufferAttachmentType::Color):
				attachmentRef = { i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
				colorAttachments.push_back(attachmentRef);
				break;
			case(SEFramebufferAttachmentType::Depth):
				attachmentRef = { i, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
				depthAttachments.push_back(attachmentRef);
				break; 
			case(SEFramebufferAttachmentType::Resolve):
				attachmentRef = { i, attachments[i].framebufferAttachment->getImageLayout() };
				resolveAttachments.push_back(attachmentRef);
				break;
			}
		}

		subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
		if (colorAttachments.size() > 0) subpassDescription.pColorAttachments = colorAttachments.data();
		if (depthAttachments.size() > 0) subpassDescription.pDepthStencilAttachment = depthAttachments.data();
		if (resolveAttachments.size() > 0) subpassDescription.pResolveAttachments = resolveAttachments.data();

		// Use subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies{};

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].srcAccessMask |= colorAttachmentLoadCount > 0 ? VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT : 0;
		dependencies[0].srcAccessMask |= depthAttachmentLoadCount > 0 ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT : 0;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create the actual renderpass
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(seDevice.getDevice(), &renderPassInfo, nullptr, &renderpass) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create render pass");
		}

		clearValues.resize(attachments.size());
		for (int i = 0; i < attachments.size(); i++) {
			switch (attachments[i].framebufferAttachment->getAttachmentType()) {
			case(SEFramebufferAttachmentType::Color):
				clearValues[i].color = attachments[i].clear.color;
				break;
			case(SEFramebufferAttachmentType::Depth):
				clearValues[i].depthStencil = attachments[i].clear.depth;
				break;
			// Resolve attachments must be loaded with VK_LOAD_OP_DONT_CARE
			}
		}
	}

	SERenderPass::~SERenderPass() {
		vkDestroyRenderPass(seDevice.getDevice(), renderpass, nullptr);
	}

	void SERenderPass::beginRenderPass(VkCommandBuffer commandbuffer, SEFramebuffer* framebuffer) {
		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderpass;
		renderPassBeginInfo.framebuffer = framebuffer->getFramebuffer();
		renderPassBeginInfo.renderArea.extent.width = framebuffer->getDimensions().x;
		renderPassBeginInfo.renderArea.extent.height = framebuffer->getDimensions().y;
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();
	
		vkCmdBeginRenderPass(commandbuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(framebuffer->getDimensions().x);
		viewport.height = static_cast<float>(framebuffer->getDimensions().y);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandbuffer, 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.extent.width = framebuffer->getDimensions().x;
		scissor.extent.height = framebuffer->getDimensions().y;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(commandbuffer, 0, 1, &scissor);
	}

	void SERenderPass::endRenderPass(VkCommandBuffer commandbuffer) {
		vkCmdEndRenderPass(commandbuffer);
	}
}