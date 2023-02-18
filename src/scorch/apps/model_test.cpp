#include "model_test.h"

#include <scorch/systems/rendering/forward_render_system.h>
#include <scorch/systems/resource_system.h>
#include <scorch/renderer/camera.h>

namespace ScorchEngine::Apps {
	ModelTest::ModelTest(const char* name) : VulkanBaseApp(name)
	{
	}
	ModelTest::~ModelTest()
	{
	}
	void ModelTest::run() {
		ResourceSystem* resourceSystem = new ResourceSystem();

		level = std::make_shared<Level>();
		Actor actor = level->createActor("mesh");
		actor.addComponent<Components::MeshComponent>().mesh = resourceSystem->loadModel(seDevice, "res/models/cube.fbx");


		RenderSystem* renderSystem = new ForwardRenderSystem(seDevice, { 1280, 720, 1 }, globalUBODescriptorLayout->getDescriptorSetLayout(), sceneSSBODescriptorLayout->getDescriptorSetLayout(), VK_SAMPLE_COUNT_1_BIT, seSwapChain->getRenderPass());
		


		SECamera camera{};
		camera.setPerspectiveProjection(70, 1.0f, 0.1f, 32.f);
		camera.setViewYXZ({ 0.f, -1.f, 0.0f }, {});

		std::vector<std::unique_ptr<SECommandBuffer>> commandBuffers{};
		commandBuffers.resize(seSwapChain->getImageCount());
		for (auto& cb : commandBuffers) {
			cb = std::make_unique<SECommandBuffer>(seDevice);
		}
		while (!seWindow.shouldClose()) {
			glfwPollEvents();

			uint32_t frameIndex = seSwapChain->getImageIndex();
			if (VkResult result = seSwapChain->acquireNextImage(&frameIndex); result == VK_SUCCESS) {
				VkCommandBuffer commandBuffer = commandBuffers[frameIndex]->getCommandBuffer();
				FrameInfo frameInfo{};
				frameInfo.level = level;
				frameInfo.frameIndex = frameIndex;
				frameInfo.globalUBO = renderData[frameIndex].uboDescriptorSet;
				frameInfo.sceneSSBO = renderData[frameIndex].ssboDescriptorSet;
				frameInfo.commandBuffer = commandBuffer; 
				frameInfo.resourceSystem = resourceSystem;

				GlobalUBO ubo{};
				ubo.viewMatrix = camera.getView();
				ubo.invViewMatrix = camera.getInverseView();
				ubo.projMatrix = camera.getProjection();
				ubo.invProjMatrix = camera.getInverseProjection();

				renderData[frameIndex].uboBuffer->writeToBuffer(&ubo);
				renderData[frameIndex].uboBuffer->flush();

				commandBuffers[frameIndex]->begin();
				seSwapChain->beginRenderPass(commandBuffer);
				renderSystem->renderOpaque(frameInfo);
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
				renderSystem->resize(glm::ivec3(extent.width, extent.height, 1));
				seWindow.resetWindowResizedFlag();
			}
		}
		vkDeviceWaitIdle(seDevice.getDevice());
		delete resourceSystem;
		delete renderSystem;
	}
}