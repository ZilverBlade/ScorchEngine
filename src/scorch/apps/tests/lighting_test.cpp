#include "lighting_test.h"

#include <chrono>
#include <scorch/renderer/camera.h>
#include <scorch/systems/rendering/forward_render_system.h>
#include <scorch/systems/rendering/light_system.h>
#include <scorch/systems/resource_system.h>

#include <scorch/systems/post_fx/fx/ppfx_screen_correct.h>

#include <scorch/controllers/camera_controller.h>

namespace ScorchEngine::Apps {
	LightingTest::LightingTest(const char* name) : VulkanBaseApp(name)
	{
	}
	LightingTest::~LightingTest()
	{
	}
	void LightingTest::run() {
		glm::vec2 resolution = { 1280, 720 };
		ResourceSystem* resourceSystem = new ResourceSystem(seDevice);

		level = std::make_shared<Level>();
		Actor actor = level->createActor("mesh");
		actor.addComponent<Components::MeshComponent>().mesh = resourceSystem->loadModel("res/models/cube.fbx");
		Actor cameraActor = level->createActor("camera actor");

		Actor lightActor = level->createActor("sun");
		lightActor.addComponent<Components::DirectionalLightComponent>();
		lightActor.getTransform().rotation.x = 0.5f;
		lightActor.getTransform().rotation.z = 0.25f;

		RenderSystem* renderSystem = new ForwardRenderSystem(
			seDevice, 
			resolution, 
			globalUBODescriptorLayout->getDescriptorSetLayout(), 
			sceneSSBODescriptorLayout->getDescriptorSetLayout(), 
			VK_SAMPLE_COUNT_8_BIT
		);
		LightSystem lightSystem = LightSystem();

		PostFX::Effect* screenCorrection = new PostFX::ScreenCorrection(
			seDevice, 
			resolution, 
			*staticPool, 
			renderSystem->getColorAttachment(), 
			seSwapChain
		);

		SECamera camera{};
		camera.setPerspectiveProjection(70.0f, 1.0f, 0.1f, 32.f);
		camera.setViewYXZ({ 0.f, -1.f, 0.0f }, {});

		std::vector<std::unique_ptr<SECommandBuffer>> commandBuffers{};
		commandBuffers.resize(seSwapChain->getImageCount());
		for (auto& cb : commandBuffers) {
			cb = std::make_unique<SECommandBuffer>(seDevice);
		}

		Controllers::CameraController controller{};

		auto oldTime = std::chrono::high_resolution_clock::now();
		while (!seWindow.shouldClose()) {
			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - oldTime).count();
			oldTime = newTime;
			glfwPollEvents();

			controller.moveTranslation(seWindow, frameTime, cameraActor);
			controller.moveOrientation(seWindow, frameTime, cameraActor);

			//actor.getTransform().rotation.z += frameTime;

			camera.setViewYXZ(cameraActor.getTransform().translation, cameraActor.getTransform().rotation);
			camera.setPerspectiveProjection(70.0f, seSwapChain->extentAspectRatio(), 0.01f, 128.f);

			uint32_t frameIndex = seSwapChain->getImageIndex();
			if (VkResult result = seSwapChain->acquireNextImage(&frameIndex); result == VK_SUCCESS) {
				VkCommandBuffer commandBuffer = commandBuffers[frameIndex]->getCommandBuffer();
				commandBuffers[frameIndex]->begin();

				FrameInfo frameInfo{};
				frameInfo.level = level;
				frameInfo.frameTime = frameTime;
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
				ubo.viewProjMatrix = ubo.projMatrix * ubo.viewMatrix;

				renderData[frameIndex].uboBuffer->writeToBuffer(&ubo);
				renderData[frameIndex].uboBuffer->flush();

				lightSystem.update(frameInfo, *renderData[frameIndex].sceneSSBO);

				renderData[frameIndex].ssboBuffer->writeToBuffer(renderData[frameIndex].sceneSSBO.get());
				renderData[frameIndex].ssboBuffer->flush();

				renderSystem->renderEarlyDepth(frameInfo);

				renderSystem->beginOpaquePass(frameInfo);
				renderSystem->renderOpaque(frameInfo);
				renderSystem->endOpaquePass(frameInfo);

				seSwapChain->beginRenderPass(commandBuffer);
				screenCorrection->render(frameInfo);
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
				renderSystem->resize({ extent.width, extent.height });
				screenCorrection->resize({ extent.width, extent.height }, {renderSystem->getColorAttachment()});
				seWindow.resetWindowResizedFlag();
			}
		}
		vkDeviceWaitIdle(seDevice.getDevice());

		delete renderSystem;
		delete screenCorrection;

		delete resourceSystem;
	}
}