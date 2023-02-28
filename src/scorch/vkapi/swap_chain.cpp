#include "swap_chain.h"
#include <simple_ini.h>
#include <array>

namespace ScorchEngine {
	SESwapChain::SESwapChain(SEDevice& device, SEWindow& window, VkExtent2D windowExtent, SESwapChain* oldSwapChain) : seDevice(device), seWindow(window), swapChainExtent(windowExtent), oldSwapChain(oldSwapChain){
		init();
	}
	SESwapChain::~SESwapChain() {
		for (size_t i = 0; i < imageCount; i++) {
			vkDestroySemaphore(seDevice.getDevice(), renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(seDevice.getDevice(), imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(seDevice.getDevice(), inFlightFences[i], nullptr);
		}
		for (VkFramebuffer frameBuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(seDevice.getDevice(), frameBuffer, nullptr);
		}
		for (VkImageView imageView : swapChainImageViews) {
			vkDestroyImageView(seDevice.getDevice(), imageView, nullptr);
		}
		vkDestroyRenderPass(seDevice.getDevice(), swapChainRenderPass, nullptr);
		vkDestroySwapchainKHR(seDevice.getDevice(), swapChain, nullptr);
	}
	void SESwapChain::init() {
		SwapChainSupportDetails details = seDevice.getSwapChainSupport(seWindow.getSurface());
		setImageCount(details.capabilities);
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(details.formats);
		swapChainImageFormat = surfaceFormat.format;
		createSwapChain(details.capabilities, getSwapChainExtent(), surfaceFormat, chooseSwapPresentMode(details.presentModes));

		createImageViews();
		createRenderPass();
		createFramebuffers();
		createSyncObjects();
	}
	VkResult SESwapChain::acquireNextImage(uint32_t* imageIndex) {
		vkWaitForFences(
			seDevice.getDevice(),
			1,
			&inFlightFences[currentFrame],
			VK_TRUE,
			UINT64_MAX
		);

		VkResult result = vkAcquireNextImageKHR(
			seDevice.getDevice(),
			swapChain,
			UINT64_MAX,
			imageAvailableSemaphores[currentFrame],  // must be a not signaled semaphore
			VK_NULL_HANDLE,
			imageIndex
		);
		return result;
	}
	VkSubmitInfo SESwapChain::getSubmitInfo(uint32_t* imageIndex) {
		if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(seDevice.getDevice(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
		}
		imagesInFlight[*imageIndex] = inFlightFences[currentFrame];
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		static VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

		vkResetFences(seDevice.getDevice(), 1, &inFlightFences[currentFrame]);
		return submitInfo;
	}
	VkResult SESwapChain::present(VkQueue queue, uint32_t* imageIndex) {
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
		
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapChain;

		presentInfo.pImageIndices = imageIndex;

		VkResult result = vkQueuePresentKHR(queue, &presentInfo);
		currentFrame = (currentFrame + 1) % imageCount;
		return result;
	}
	void SESwapChain::createSwapChain(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D extent, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode) {
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = seWindow.getSurface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.preTransform = capabilities.currentTransform;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		if (oldSwapChain) {
			createInfo.oldSwapchain = oldSwapChain->swapChain;
		} else {
			createInfo.oldSwapchain = nullptr;
		}
		QueueFamilyIndices indices = seDevice.findPhysicalQueueFamilies();
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;      // Optional
			createInfo.pQueueFamilyIndices = nullptr;  // Optional
		}
		if (vkCreateSwapchainKHR(seDevice.getDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}
	}
	void SESwapChain::createImageViews() {
		std::vector<VkImage> swapChainImages{};
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(seDevice.getDevice(), swapChain, &imageCount, swapChainImages.data());

		for (int i = 0; i < imageCount; i++) {
			VkImageViewCreateInfo imageViewCreateInfo = {};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.format = swapChainImageFormat;
			VkImageSubresourceRange subresourceRange{};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = 1;
			subresourceRange.baseArrayLayer = 0;
			subresourceRange.layerCount = 1;
			imageViewCreateInfo.subresourceRange = subresourceRange;
			imageViewCreateInfo.image = swapChainImages[i];
			VkImageView imageView;
			if (vkCreateImageView(seDevice.getDevice(), &imageViewCreateInfo, nullptr, &imageView) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create image view!");
			}
			swapChainImageViews.push_back(imageView);
		}
	}
	void SESwapChain::createRenderPass() {
		std::array<VkAttachmentDescription, 1> attachmentDescriptions{};
	
		attachmentDescriptions[0].format = swapChainImageFormat;
		attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		std::array<VkAttachmentReference, 1> colorAttachments{};
		
		colorAttachments[0] = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkSubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.pColorAttachments = colorAttachments.data();
		subpassDescription.colorAttachmentCount = colorAttachments.size();
		subpassDescription.pDepthStencilAttachment = nullptr;
		subpassDescription.pResolveAttachments = nullptr;

		// Use subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 1> dependencies{};

		dependencies[0].dstSubpass = 0;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].srcAccessMask = 0;
		dependencies[0].srcStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		// Create the actual renderpass
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(seDevice.getDevice(), &renderPassInfo, nullptr, &swapChainRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create render pass");
		}
	}
	void SESwapChain::createFramebuffers() {
		for (int i = 0; i < imageCount; i++) {
			VkFramebufferCreateInfo fbufferCreateInfo{};
			fbufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbufferCreateInfo.pNext = nullptr;
			fbufferCreateInfo.flags = 0;
			fbufferCreateInfo.renderPass = swapChainRenderPass;
			fbufferCreateInfo.attachmentCount = 1;
			fbufferCreateInfo.pAttachments = &swapChainImageViews[i];
			fbufferCreateInfo.width = swapChainExtent.width;
			fbufferCreateInfo.height = swapChainExtent.height;
			fbufferCreateInfo.layers = 1;
			VkFramebuffer framebuffer;
			if (vkCreateFramebuffer(seDevice.getDevice(), &fbufferCreateInfo, nullptr, &framebuffer) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create framebuffer!");
			}
			swapChainFramebuffers.push_back(framebuffer);
		}
	}
	void SESwapChain::createSyncObjects() {
		imageAvailableSemaphores.resize(imageCount);
		renderFinishedSemaphores.resize(imageCount);
		inFlightFences.resize(imageCount);
		imagesInFlight.resize(imageCount, nullptr);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < imageCount; i++) {
			if (vkCreateSemaphore(seDevice.getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(seDevice.getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(seDevice.getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	
	}

	void SESwapChain::setImageCount(const VkSurfaceCapabilitiesKHR& capabilities) {
		CSimpleIniA ini{};
		ini.LoadFile("res/config/scorch.ini");
		imageCount = ini.GetLongValue("DISPLAY", "SwapChainImageCount");

		imageCount = std::clamp(imageCount, capabilities.minImageCount, capabilities.maxImageCount);
	}


	VkSurfaceFormatKHR SESwapChain::chooseSwapSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		CSimpleIniA ini{};
		ini.LoadFile("res/config/scorch.ini");
		bool highbitcol = ini.GetBoolValue("DISPLAY", "SwapChain10Bit");

		VkSurfaceFormatKHR finalFormat{};
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				finalFormat = availableFormat;
			}
			if (availableFormat.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && highbitcol) {
				finalFormat = availableFormat;
				break;
			}
		}
		if (finalFormat.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32) {
			SELOG_INF("Using 10 bit color format");
		} else {
			SELOG_INF("Using 8 bit color format");
		}
		return finalFormat;
	}

	VkPresentModeKHR SESwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		CSimpleIniA ini{};
		ini.LoadFile("res/config/scorch.ini");
		bool vsync = ini.GetBoolValue("DISPLAY", "VerticalSync");
		if (vsync == false) {
			for (const auto& availablePresentMode : availablePresentModes) {
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
					SELOG_INF("Present mode: Mailbox");
					return VK_PRESENT_MODE_MAILBOX_KHR;
				} else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
					SELOG_INF("Present mode: Immediate");
					return VK_PRESENT_MODE_IMMEDIATE_KHR;
				}
			}
		} else {
			SELOG_INF("Present mode: V-Sync");
			return VK_PRESENT_MODE_FIFO_KHR;
		}
		SELOG_ERR("Invalid present mode!");
	}

	VkExtent2D SESwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		} else {
			VkExtent2D actualExtent = windowExtent;
			actualExtent.width = std::max(
				capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(
				capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}
}