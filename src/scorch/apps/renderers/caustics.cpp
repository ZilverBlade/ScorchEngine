#include "caustics.h"

#include <chrono>
#include <scorch/rendering/camera.h>
#include <scorch/systems/rendering/forward_render_system.h>
#include <scorch/systems/rendering/light_system.h>
#include <scorch/systems/rendering/sky_light_system.h>
#include <scorch/systems/rendering/skybox_system.h>
#include <scorch/systems/resource_system.h>

#include <scorch/systems/post_fx/fx/ppfx_screen_correct.h>

#include <scorch/controllers/camera_controller.h>
#include <scorch/graphics/surface_material.h>
#include <scorch/systems/rendering/shadow_map_system.h>

namespace ScorchEngine::Apps {
	CausticsApp::CausticsApp(const char* name) : VulkanBaseApp(name)
	{
	}
	CausticsApp::~CausticsApp()
	{
	}
	void CausticsApp::run() {
		glm::vec2 resolution = { 1280, 720 };
		ResourceSystem* resourceSystem = new ResourceSystem(seDevice, *staticPool);
		MaterialSystem::createDescriptorSetLayout(seDevice);
		std::unique_ptr<SEDescriptorSetLayout> skyLightDescriptorLayout = SEDescriptorSetLayout::Builder(seDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // prefiltered
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // irradiance
			.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // brdfLUT
			.build();
	
		ShadowMapSystem* shadowMapSystem = new ShadowMapSystem(seDevice, *staticPool);

		VkSampleCountFlagBits msaa = VK_SAMPLE_COUNT_8_BIT;
		RenderSystem* renderSystem = new ForwardRenderSystem(
			seDevice,
			resolution, {
				globalUBODescriptorLayout->getDescriptorSetLayout(),
				sceneDescriptorLayout->getDescriptorSetLayout(),
				skyLightDescriptorLayout->getDescriptorSetLayout(),
				shadowMapSystem->getDescriptorSetLayout()
			},
			msaa
		);
		SkyLightSystem* skyLightSystem = new SkyLightSystem(
			seDevice,
			*staticPool,
			skyLightDescriptorLayout,
			seSwapChain->getImageCount()
		);
		SkyboxSystem* skyboxSystem = new SkyboxSystem(
			seDevice,
			renderSystem->getOpaqueRenderPass()->getRenderPass(),
			globalUBODescriptorLayout->getDescriptorSetLayout(),
			sceneDescriptorLayout->getDescriptorSetLayout(),
			skyLightDescriptorLayout->getDescriptorSetLayout(),
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

		std::vector<std::unique_ptr<SECommandBuffer>> commandBuffers{};
		commandBuffers.resize(seSwapChain->getImageCount());
		for (auto& cb : commandBuffers) {
			cb = std::make_unique<SECommandBuffer>(seDevice);
		}

		Controllers::CameraController controller{};


		SECamera camera{};
		camera.setPerspectiveProjection(70.0f, 1.0f, 0.1f, 32.f);

		ResourceID missingMaterial = resourceSystem->loadSurfaceMaterial("res/materials/missing_material.json");
		ResourceID blankMaterial = resourceSystem->loadSurfaceMaterial("res/materials/blank.json");
		ResourceID glassMaterial = resourceSystem->loadSurfaceMaterial("res/materials/glass.json");
		level = std::make_shared<Level>();
		Actor cameraActor = level->createActor("cameraActor");
		Actor lightActor = level->createActor("sun");
		Actor sphereActor = level->createActor("sphereActor");
		Actor floorActor = level->createActor("floorActor");
		{

			auto& floorM = floorActor.addComponent<MeshComponent>();
			floorM.mesh = resourceSystem->loadModel("res/models/sphere.fbx");
			for (const std::string& mapto : resourceSystem->getModel(floorM.mesh)->getSubmeshes()) {
				floorM.materials[mapto] = blankMaterial;
			}
			floorActor.getTransform().translation = { 0.f, 0.f, 2.0f };
			floorActor.getTransform().scale = { 50.0, 50.0, 0.01f };

			auto& msc2 = sphereActor.addComponent<MeshComponent>();
			msc2.mesh = resourceSystem->loadModel("res/models/cylinder.fbx");
			for (const std::string& mapto : resourceSystem->getModel(msc2.mesh)->getSubmeshes()) {
				msc2.materials[mapto] = glassMaterial;
			}
			sphereActor.getTransform().translation = { 0.f, 0.f, 3.0f };

			{
				Actor meshactor = level->createActor("some kind of mesh");
				auto& meshactorm = meshactor.addComponent<MeshComponent>();
				meshactorm.mesh = resourceSystem->loadModel("res/models/sphere.fbx");
				for (const std::string& mapto : resourceSystem->getModel(meshactorm.mesh)->getSubmeshes()) {
					meshactorm.materials[mapto] = glassMaterial;
				}
				meshactor.getTransform().translation = { 0.f, 3.f, 3.0f };
			}
			{
				Actor meshactor = level->createActor("some kind of mesh");
				auto& meshactorm = meshactor.addComponent<MeshComponent>();
				meshactorm.mesh = resourceSystem->loadModel("res/models/teapot.fbx");
				for (const std::string& mapto : resourceSystem->getModel(meshactorm.mesh)->getSubmeshes()) {
					meshactorm.materials[mapto] = glassMaterial;
				}
				meshactor.getTransform().translation = { 4.f, 3.f, 3.0f };
				meshactor.getTransform().rotation = { -1.51, 0.0, 0.0 };
				meshactor.getTransform().scale = { 100.0, 100.0, 100.0 };
			}

			Actor skyboxActor = level->createActor("skyboxActor");
			auto& sbc = skyboxActor.addComponent<SkyboxComponent>();
			auto& slc = skyboxActor.addComponent<SkyLightComponent>();
			slc.environmentMap = resourceSystem->loadTextureCube("res/environmentmaps/skywater").id;
			slc.intensity = 0.60f;
			cameraActor.getTransform().translation = { 0.f, -1.f, 2.0f };
			{
				lightActor.addComponent<DirectionalLightComponent>();
				lightActor.addComponent<LightPropagationVolumeComponent>();
				lightActor.getTransform().rotation.x = 0.54f;
				lightActor.getTransform().rotation.z = 0.25f;
				lightActor.getComponent<DirectionalLightComponent>().intensity = 4.0f;
				lightActor.getComponent<DirectionalLightComponent>().shadow.maxDistance = 16.0f;
				lightActor.getComponent<LightPropagationVolumeComponent>().maxExtent = { 4, 4, 4 };
				lightActor.getComponent<LightPropagationVolumeComponent>().cascadeCount = 1;
				lightActor.getComponent<LightPropagationVolumeComponent>().propagationIterations = 1;
			}
		}

		float flPropCount = 2.0f;
		float incrementTime = 0;
		auto oldTime = std::chrono::high_resolution_clock::now();
		while (!seWindow.shouldClose()) {
			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - oldTime).count();
			oldTime = newTime;
			glfwPollEvents();

			controller.moveTranslation(seWindow, frameTime, cameraActor);
			controller.moveOrientation(seWindow, frameTime, cameraActor);

			if (seWindow.isKeyDown(GLFW_KEY_LEFT)) {
				lightActor.getTransform().rotation.z -= frameTime;
			}
			if (seWindow.isKeyDown(GLFW_KEY_RIGHT)) {
				lightActor.getTransform().rotation.z += frameTime;
			}
			if (seWindow.isKeyDown(GLFW_KEY_UP)) {
				lightActor.getTransform().rotation.x += frameTime;
			}
			if (seWindow.isKeyDown(GLFW_KEY_DOWN)) {
				lightActor.getTransform().rotation.x -= frameTime;
			}

			camera.setViewYXZ(cameraActor.getTransform().translation, cameraActor.getTransform().rotation);
			camera.setPerspectiveProjection(70.0f, seSwapChain->extentAspectRatio(), 0.01f, 128.f);


			uint32_t frameIndex = seSwapChain->getImageIndex();
			if (VkResult result = seSwapChain->acquireNextImage(&frameIndex); result == VK_SUCCESS) {
				VkCommandBuffer commandBuffer = commandBuffers[frameIndex]->getCommandBuffer();
				commandBuffers[frameIndex]->begin();

				FrameInfo frameInfo{};
				frameInfo.level = level;
				frameInfo.camera = camera;
				frameInfo.frameTime = frameTime;
				frameInfo.frameIndex = frameIndex;
				frameInfo.globalUBO = renderData[frameIndex].uboDescriptorSet;
				frameInfo.sceneSSBO = renderData[frameIndex].ssboDescriptorSet;
				frameInfo.shadowMap = shadowMapSystem->getDescriptorSet();
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

				shadowMapSystem->render(frameInfo);

				lightSystem.update(frameInfo, *renderData[frameIndex].sceneSSBO);
				SETextureCube* pickedCube = nullptr;
				frameInfo.level->getRegistry().view<Components::SkyLightComponent>().each(
					[&](auto& skylight) {
						pickedCube = resourceSystem->getTextureCube({ skylight.environmentMap, true, true });
					}
				);
				if (pickedCube) {
					skyLightSystem->update(frameInfo, *renderData[frameIndex].sceneSSBO, pickedCube);
				}

				renderData[frameIndex].ssboBuffer->writeToBuffer(renderData[frameIndex].sceneSSBO.get());
				renderData[frameIndex].ssboBuffer->flush();

				renderSystem->renderEarlyDepth(frameInfo);

				renderSystem->beginOpaquePass(frameInfo);
				frameInfo.skyLight = skyLightSystem->getDescriptorSet(frameIndex);
				renderSystem->renderOpaque(frameInfo);
				if (pickedCube) {
					skyboxSystem->render(frameInfo, skyLightSystem->getDescriptorSet(frameIndex));
				}
				renderSystem->renderTranslucent(frameInfo);
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
				screenCorrection->resize({ extent.width, extent.height }, { renderSystem->getColorAttachment() });
				seWindow.resetWindowResizedFlag();
			}
		}
		vkDeviceWaitIdle(seDevice.getDevice());

		MaterialSystem::destroyDescriptorSetLayout();
		delete renderSystem;
		delete screenCorrection;
		delete skyLightSystem;
		delete skyboxSystem;

		delete resourceSystem;
	}
}