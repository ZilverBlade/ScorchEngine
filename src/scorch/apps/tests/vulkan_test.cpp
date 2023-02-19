#include "vulkan_test.h"

#include <scorch/vkapi/graphics_pipeline.h>
#include <scorch/vkapi/pipeline_layout.h>
#include <scorch/vkapi/descriptors.h>

namespace ScorchEngine::Apps {
	VulkanTest::VulkanTest(const char* name) : 
		App(name), 
		seRenderer(new SERenderer(seDevice)), 
		seSwapChain(new SESwapChain(seDevice, seWindow, seWindow.getExtent()))
	{
	}
	VulkanTest::~VulkanTest() {
		delete seRenderer;
		delete seSwapChain;
	}
	void VulkanTest::run() {
		SEGraphicsPipelineConfigInfo pipelineConfigInfo = SEGraphicsPipelineConfigInfo();

		SEPipelineLayout* pipelineLayout = new SEPipelineLayout(seDevice);
		pipelineConfigInfo.pipelineLayout = pipelineLayout->getPipelineLayout();
		pipelineConfigInfo.renderPass = seSwapChain->getRenderPass()->getRenderPass();
		//pipelineConfigInfo.wireframe(1.f);

		SEGraphicsPipeline* pipeline = new SEGraphicsPipeline(
			seDevice,
			{ SEShader(SEShaderType::Vertex, "res/shaders/spirv/triangle.vsh.spv"), SEShader(SEShaderType::Fragment, "res/shaders/spirv/triangle.fsh.spv") },
			pipelineConfigInfo
		);

		std::vector<std::unique_ptr<SECommandBuffer>> commandBuffers{};
		commandBuffers.resize(seSwapChain->getImageCount());
		for (auto& cb : commandBuffers) {
			cb = std::make_unique<SECommandBuffer>(seDevice);
		}
		while (!seWindow.shouldClose()) {
			glfwPollEvents();
			
			uint32_t frameIndex = seSwapChain->getImageIndex();
			if (VkResult result = seSwapChain->acquireNextImage(&frameIndex); result == VK_SUCCESS) {
				commandBuffers[frameIndex]->begin();
				VkCommandBuffer commandBuffer = commandBuffers[frameIndex]->getCommandBuffer();
				seSwapChain->beginRenderPass(commandBuffer);

				pipeline->bind(commandBuffer);
				vkCmdDraw(commandBuffer, 3, 1, 0, 0);

				seSwapChain->endRenderPass(commandBuffer);
				commandBuffers[frameIndex]->end();

				seRenderer->submitCommandBuffers({ commandBuffers[frameIndex].get() });	
				seRenderer->submitQueue(seSwapChain->getSubmitInfo(&frameIndex), seSwapChain->getFence(frameIndex));
				seSwapChain->present(seRenderer->getQueue(), &frameIndex);
			} else if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
				VkExtent2D extent = seWindow.getExtent();

				while (extent.width == 0 || extent.height == 0) {
					extent = seWindow.getExtent();
					glfwWaitEvents();
				}
				vkDeviceWaitIdle(seDevice.getDevice());
				SESwapChain* oldSwapChain = seSwapChain;
				seSwapChain = new SESwapChain(seDevice, seWindow, extent, oldSwapChain);
				delete oldSwapChain;
				seWindow.resetWindowResizedFlag();
			}
		}
		vkDeviceWaitIdle(seDevice.getDevice());
		delete pipeline;
		delete pipelineLayout;
	}
}