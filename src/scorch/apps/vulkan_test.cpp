#include <scorch/apps/vulkan_test.h>
#include <ghudcpp/ghud.h>
#include <ghudvk/vk_context.h>

namespace ScorchEngine {
	VulkanTest::VulkanTest(const char* name) : 
		App(name), 
		seRenderer(seDevice), 
		seSwapChain(seDevice, seWindow, seWindow.getExtent()) 
	{
	}
	VulkanTest::~VulkanTest() {}
	void VulkanTest::run() {

		GHUD::VulkanContextCreateInfo ghudvkCreateInfo{};
		ghudvkCreateInfo.m_MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		ghudvkCreateInfo.m_Device = seDevice.getDevice();
		ghudvkCreateInfo.m_PhysicalDevice = seDevice.getPhysicalDevice();
		ghudvkCreateInfo.m_FrameBufferFormat = seSwapChain.getSwapChainImageFormat();
		ghudvkCreateInfo.m_RenderPass = seSwapChain.getRenderPass();
		ghudvkCreateInfo.m_SubPass = 0;
		ghudvkCreateInfo.m_SwapChainImageCount = seSwapChain.getImageCount();
		GHUD::VulkanContext* ghud = new GHUD::VulkanContext(ghudvkCreateInfo);

		VkCommandBuffer stc = seDevice.beginSingleTimeCommands();
		ghud->CreateResources(stc);
		seDevice.endSingleTimeCommands(stc);

		GHUD::DrawList* drawList = ghud->GetDrawList();

		GHUD::Element::Line lineA{};
		lineA.m_Color = 0xFFFF00FF;
		lineA.m_PointA = { 0.5f, 0.5f };
		lineA.m_PointB = { 0.5f, 0.5f };
		lineA.m_Width = 2.0;
		lineA.m_Layer = 2;
		GHUD::Element::Line lineB{};
		lineB.m_Color = 0x00FFFFFF;
		lineB.m_PointA = { 0.5f, 0.5f };
		lineB.m_PointB = { 1.0f, 0.5f };
		lineB.m_Width = 2.0;
		lineB.m_Layer = 1;

		ghud->Resize({ static_cast<float>(seWindow.getExtent().width),  static_cast<float>(seWindow.getExtent().height) });		

		std::vector<std::unique_ptr<SECommandBuffer>> commandBuffers{};
		commandBuffers.resize(seSwapChain.getImageCount());
		for (auto& cb : commandBuffers) {
			cb = std::make_unique<SECommandBuffer>(seDevice);
		}
		while (!seWindow.shouldClose()) {
			glfwPollEvents();
			

			uint32_t frameIndex = seSwapChain.getImageIndex();
			if (seSwapChain.acquireNextImage(&frameIndex) == VK_SUCCESS) {

				drawList->Clear();
				drawList->FrameStart();
				drawList->DrawLine(lineA);
				drawList->DrawLine(lineB);
				drawList->FrameEnd();

				commandBuffers[frameIndex]->begin();

				// render offscreen

				seSwapChain.beginRenderPass(commandBuffers[frameIndex]->getCommandBuffer());

				// render to swapchain
				GHUD::VulkanFrameInfo frameInfo{};
				frameInfo.m_CommandBuffer = commandBuffers[frameIndex]->getCommandBuffer();
				frameInfo.m_FrameIndex = frameIndex;

				ghud->Render(&frameInfo);

				seSwapChain.endRenderPass(commandBuffers[frameIndex]->getCommandBuffer());
				commandBuffers[frameIndex]->end();

				seRenderer.submitCommandBuffers({ commandBuffers[frameIndex].get() });	
				seRenderer.submitQueue(seSwapChain.getSubmitInfo(&frameIndex), seSwapChain.getFence(frameIndex));
				seSwapChain.present(seRenderer.getQueue(), &frameIndex);
			}
		}
		vkDeviceWaitIdle(seDevice.getDevice());

		delete ghud;
	}
}