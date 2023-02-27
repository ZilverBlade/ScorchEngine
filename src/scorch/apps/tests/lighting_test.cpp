#include "lighting_test.h"

#include <chrono>
#include <scorch/rendering/camera.h>
#include <scorch/systems/rendering/forward_render_system.h>
#include <scorch/systems/rendering/light_system.h>
#include <scorch/systems/rendering/skybox_system.h>
#include <scorch/systems/resource_system.h>

#include <scorch/systems/post_fx/fx/ppfx_screen_correct.h>

#include <scorch/controllers/camera_controller.h>
#include <scorch/graphics/surface_material.h>

namespace ScorchEngine::Apps {
	LightingTest::LightingTest(const char* name) : VulkanBaseApp(name)
	{
	}
	LightingTest::~LightingTest()
	{
	}
	void LightingTest::run() {
		glm::vec2 resolution = { 1280, 720 };
		ResourceSystem* resourceSystem = new ResourceSystem(seDevice, *staticPool);
		MaterialSystem::createDescriptorSetLayout(seDevice);

		ResourceID missingMaterial = resourceSystem->loadSurfaceMaterial("res/materials/missing_material.json");
		ResourceID clearCoatMaterial = resourceSystem->loadSurfaceMaterial("res/materials/paint.json");
		ResourceID blankMaterial = resourceSystem->loadSurfaceMaterial("res/materials/blank.json");

		level = std::make_shared<Level>();
		Actor floorActor = level->createActor("floor");
		auto& msc = floorActor.addComponent<Components::MeshComponent>();
		msc.mesh = resourceSystem->loadModel("res/models/cube.fbx");
		for (const std::string& mapto : resourceSystem->getModel(msc.mesh)->getSubmeshes()) {
			msc.materials[mapto] = blankMaterial;
		}
		floorActor.getTransform().scale = { 20.f, 20.f, 1.0f };
		Actor sphereActor = level->createActor("sphereActor");
		auto& msc2 = sphereActor.addComponent<Components::MeshComponent>();
		msc2.mesh = resourceSystem->loadModel("res/models/sphere.fbx");
		for (const std::string& mapto : resourceSystem->getModel(msc2.mesh)->getSubmeshes()) {
			msc2.materials[mapto] = clearCoatMaterial;
		}
		sphereActor.getTransform().translation = { 0.f, 0.f, 3.0f };

		Actor skyboxActor = level->createActor("skyboxActor");
		auto& sbc = skyboxActor.addComponent<Components::SkyboxComponent>();
		sbc.environmentMap = resourceSystem->loadTextureCube("res/environmentmaps/skywater").id;

		Actor cameraActor = level->createActor("camera actor");
		cameraActor.getTransform().translation = { 0.f, -1.f, 2.0f };
		{
			Actor lightActor = level->createActor("sun");
			lightActor.addComponent<Components::DirectionalLightComponent>();
			lightActor.getTransform().rotation.x = 0.5f;
			lightActor.getTransform().rotation.z = 0.25f;
		}
		{
			cameraActor.addComponent<Components::PointLightComponent>().emission = {1.0, 0.0, 0.0};
			cameraActor.getTransform().translation = { 1.0, 1.0, 3.0f};
		}
		VkSampleCountFlagBits msaa = VK_SAMPLE_COUNT_8_BIT;
		auto skyboxDescriptorLayout = SEDescriptorSetLayout::Builder(seDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();
		RenderSystem* renderSystem = new ForwardRenderSystem(
			seDevice, 
			resolution, { 
				globalUBODescriptorLayout->getDescriptorSetLayout(),
				sceneSSBODescriptorLayout->getDescriptorSetLayout(),
				skyboxDescriptorLayout->getDescriptorSetLayout()
			},
			msaa
		);
		SkyboxSystem* skyboxSystem = new SkyboxSystem(
			seDevice,
			*staticPool,
			*skyboxDescriptorLayout,
			renderSystem->getOpaqueRenderPass()->getRenderPass(),
			globalUBODescriptorLayout->getDescriptorSetLayout(),
			sceneSSBODescriptorLayout->getDescriptorSetLayout(),
			msaa
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

		std::vector<std::unique_ptr<SECommandBuffer>> commandBuffers{};
		commandBuffers.resize(seSwapChain->getImageCount());
		for (auto& cb : commandBuffers) {
			cb = std::make_unique<SECommandBuffer>(seDevice);
		}

		Controllers::CameraController controller{};

		float incrementTime = 0;
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

			incrementTime += frameTime * 1.0;
			resourceSystem->getSurfaceMaterial(clearCoatMaterial)->roughnessFactor = (sin(incrementTime) + 1.0) * 0.5;
			resourceSystem->getSurfaceMaterial(clearCoatMaterial)->updateParams();

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
				skyboxSystem->update(frameInfo, *renderData[frameIndex].sceneSSBO);

				renderData[frameIndex].ssboBuffer->writeToBuffer(renderData[frameIndex].sceneSSBO.get());
				renderData[frameIndex].ssboBuffer->flush();

				renderSystem->renderEarlyDepth(frameInfo);

				renderSystem->beginOpaquePass(frameInfo);
				renderSystem->renderOpaque(frameInfo, skyboxSystem->getSkyboxDescriptor());
				skyboxSystem->renderSkybox(frameInfo);
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

		MaterialSystem::destroyDescriptorSetLayout();
		delete renderSystem;
		delete screenCorrection;

		delete resourceSystem;
	}
}