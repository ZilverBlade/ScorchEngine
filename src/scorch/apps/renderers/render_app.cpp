#include "render_app.h"

#include <chrono>
#include <scorch/rendering/camera.h>
#include <scorch/systems/rendering/forward_render_system.h>
#include <scorch/systems/rendering/light_system.h>
#include <scorch/systems/rendering/sky_light_system.h>
#include <scorch/systems/rendering/skybox_system.h>
#include <scorch/systems/rendering/vsdf_previewer.h>
#include <scorch/systems/rendering/vsdf_render_system.h>
#include <scorch/systems/rendering/shadow_map_system.h>

#include <scorch/systems/resource_system.h>

#include <scorch/systems/post_fx/fx/ppfx_screen_correct.h>

#include <scorch/controllers/camera_controller.h>
#include <scorch/graphics/surface_material.h>
#include <scorch/systems/ui/performance_viewer.h>
#include <shudvk/vk_context.h>
#include <fstream>

namespace ScorchEngine::Apps {
	RenderApp::RenderApp(const char* name) : VulkanBaseApp(name)
	{
	}
	RenderApp::~RenderApp()
	{
	}
	void RenderApp::run() {
		glm::vec2 resolution = { 1280, 720 };
		ResourceSystem* resourceSystem = new ResourceSystem(seDevice, seDevice.getDescriptorPool());
		MaterialSystem::createDescriptorSetLayout(seDevice);
		std::unique_ptr<SEDescriptorSetLayout> skyLightDescriptorLayout = SEDescriptorSetLayout::Builder(seDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // prefiltered
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // irradiance
			.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // brdfLUT
			.build();

		SHUD::VulkanContextCreateInfo uiContextCreate{};
		uiContextCreate.mDevice = seDevice.getDevice();
		uiContextCreate.mFramebufferFormat = seSwapChain->getSwapChainImageFormat();
		uiContextCreate.mPhysicalDevice = seDevice.getPhysicalDevice();
		uiContextCreate.mRenderPass = seSwapChain->getRenderPass();
		uiContextCreate.mSwapChainImageCount = seSwapChain->getImageCount();
		SHUD::VulkanContext* uiContext = new SHUD::VulkanContext(uiContextCreate);

		VkCommandBuffer singleTimeCommands = seDevice.beginSingleTimeCommands();
		uiContext->CreateResources(singleTimeCommands);
		seDevice.endSingleTimeCommands(singleTimeCommands);
		uiContext->SetResolution({ (float)seWindow.getExtent().width,(float)seWindow.getExtent().height });

		UIPerformanceViewer performanceViewer = UIPerformanceViewer(seDevice);

		ShadowMapSystem* shadowMapSystem = new ShadowMapSystem(seDevice);

		VkSampleCountFlagBits msaa = VK_SAMPLE_COUNT_1_BIT;

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
		VoxelSdfPreviewer* sdfPreviewer = new VoxelSdfPreviewer(
			seDevice,
			renderSystem->getOpaqueRenderPass()->getRenderPass(),
			globalUBODescriptorLayout->getDescriptorSetLayout(),
			msaa
		);
		VoxelSdfRenderSystem* sdfRenderSystem = new VoxelSdfRenderSystem(
			seDevice,
			resolution,
			renderSystem->getDepthAttachment()->getDescriptor(),
			globalUBODescriptorLayout->getDescriptorSetLayout()
		);
		renderSystem->setSdfShadowTexture(sdfRenderSystem->getShadowMaskDescriptor());

		LightSystem lightSystem = LightSystem();

		PostFX::Effect* screenCorrection = new PostFX::ScreenCorrection(
			seDevice,
			resolution,
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
		ResourceID clearCoatMaterial = resourceSystem->loadSurfaceMaterial("res/materials/paint.json");
		ResourceID blankMaterial = resourceSystem->loadSurfaceMaterial("res/materials/blank.json");
		ResourceID glassMaterial = resourceSystem->loadSurfaceMaterial("res/materials/glass.json");

		ResourceID sphereMesh = resourceSystem->loadModel("res/models/sphere.json");
		ResourceID cylinderMesh = resourceSystem->loadModel("res/models/cylinder.json");
		ResourceID cubeMesh = resourceSystem->loadModel("res/models/cube.json");
		ResourceID teapotMesh = resourceSystem->loadModel("res/models/teapot.json");
		
		level = std::make_shared<Level>();
		Actor cameraActor = level->createActor("cameraActor");
		Actor lightActor = level->createActor("sun");
		Actor sphereActor = level->createActor("sphereActor");
		Actor floorActor = level->createActor("floorActor");
		Actor teapotActor = level->createActor("teapot");
		{

			//Actor sponzaActor = level->createActor("sponzaActor");
			//auto& msc = sponzaActor.addComponent<MeshComponent>();
			//const char* sponzaFBX = "D:/Directories/Downloads/TheRealMJP DeferredTexturing master Content-Models_Sponza/sponza.fbx";
			//msc.mesh = resourceSystem->loadModel(sponzaFBX);
			//
			//auto bdr = SEModel::Builder();
			//bdr.loadModel(sponzaFBX);
			//auto sponzaMaterials = bdr.loadMaterials(seDevice, resourceSystem);
			//for (auto& [slot, id] : *sponzaMaterials) {
			//	msc.materials[slot] = id;
			//}
			//sponzaActor.getTransform().rotation = { -3.1415926f / 2.f, 0.f, 0.f };

			//Actor carActor = level->createActor("carActor");
			//auto& msc22= carActor.addComponent<MeshComponent>();
			//const char* carFBX = "E:/sexcarsmoment/SexCars/carlist/yakisoba/bikini/models/bikini_EXPERIMENTAL.fbx";
			//msc22.mesh = resourceSystem->loadModel(carFBX);
			//
			//for (const std::string& mapto: resourceSystem->getModel(msc22.mesh)->getSubmeshes()) {
			//	msc22.materials[mapto] = clearCoatMaterial;
			//}
			//carActor.getTransform().translation = { 0.f, 0.f, 1.f };

			auto& floorM = floorActor.addComponent<MeshComponent>();
			floorM.mesh = cylinderMesh;
			for (const std::string& mapto : resourceSystem->getModel(floorM.mesh)->getSubmeshes()) {
				floorM.materials[mapto] = blankMaterial;
			}
			floorActor.getTransform().rotation = { 1.51f, 0.0f, 0.f };
			floorActor.getTransform().translation = { 0.f, -1.0f, 0.f};
			floorActor.getTransform().scale = {50.0f, 50.f, 1.0f};

			auto& msc2 = sphereActor.addComponent<MeshComponent>();
			msc2.mesh = sphereMesh;
			for (const std::string& mapto : resourceSystem->getModel(msc2.mesh)->getSubmeshes()) {
				msc2.materials[mapto] = blankMaterial;//clearCoatMaterial;
			}
			sphereActor.getTransform().translation = { 0.f, 3.0f, 0.f };

			{
				auto& meshactorm = teapotActor.addComponent<MeshComponent>();
				meshactorm.mesh = teapotMesh;
				for (const std::string& mapto : resourceSystem->getModel(meshactorm.mesh)->getSubmeshes()) {
					meshactorm.materials[mapto] = blankMaterial;
				}
				teapotActor.getTransform().translation = { 6.f, 3.f, 10.0f };
			}
			{
				Actor meshactor = level->createActor("some kind of mesh");
				auto& meshactorm = meshactor.addComponent<MeshComponent>();
				meshactorm.mesh = cubeMesh;
				for (const std::string& mapto : resourceSystem->getModel(meshactorm.mesh)->getSubmeshes()) {
					meshactorm.materials[mapto] = blankMaterial;
				}
				meshactor.getTransform().translation = { 9.f, 3.f, 0.0f };
				//meshactor.getTransform().rotation = { 0.0, 0.0, 0.0 };
				//meshactor.getTransform().scale = { 100.0, 100.0, 100.0 };
			}

			Actor skyboxActor = level->createActor("skyboxActor");
			auto& sbc = skyboxActor.addComponent<SkyboxComponent>();
			auto& slc = skyboxActor.addComponent<SkyLightComponent>();
			slc.environmentMap = resourceSystem->loadTextureCube("res/environmentmaps/skywater").id;
			slc.intensity = 0.60f;
			cameraActor.getTransform().translation = { 0.f,2.0f, -1.f};
			{
				lightActor.addComponent<DirectionalLightComponent>();
				lightActor.addComponent<LightPropagationVolumeComponent>();
				lightActor.getTransform().rotation.x = 0.54f;
				lightActor.getTransform().rotation.y = 0.25f;
				lightActor.getComponent<DirectionalLightComponent>().intensity = 4.0f;
				lightActor.getComponent<DirectionalLightComponent>().shadow.maxDistance = 16.0f;
				lightActor.getComponent<LightPropagationVolumeComponent>().maxExtent = { 32, 32, 32 }; // coverage should be large for better results
				lightActor.getComponent<LightPropagationVolumeComponent>().cascadeCount = 1;
				lightActor.getComponent<LightPropagationVolumeComponent>().propagationIterations = 2;
			}
			{
				//cameraActor.addComponent<Components::PointLightComponent>().emission = { 1.0, 0.0, 0.0 };
				//cameraActor.getTransform().translation = { 1.0, 1.0, 3.0f };
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
			uiContext->UpdateIOFrame();
			uiContext->Pick();

			performanceViewer.update(frameTime);
			uiContext->GetDrawList()->Clear();
			uiContext->GetDrawList()->FrameStart();
			performanceViewer.renderFps(uiContext->GetDrawList(), { -100.f, 100.f });
			performanceViewer.renderDtGraph(uiContext->GetDrawList(), { 50.f, 50.f }, { 400.f, 200.f });
			uiContext->GetDrawList()->FrameEnd();

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
			if (seWindow.isKeyDown(GLFW_KEY_EQUAL)) {
				lightActor.getComponent<LightPropagationVolumeComponent>().boost += glm::vec3(4.0 * frameTime);
			}
			if (seWindow.isKeyDown(GLFW_KEY_MINUS)) {
				lightActor.getComponent<LightPropagationVolumeComponent>().boost -= glm::vec3(4.0 * frameTime);
				lightActor.getComponent<LightPropagationVolumeComponent>().boost = glm::max(lightActor.getComponent<LightPropagationVolumeComponent>().boost, glm::vec3(0.0));
			}
			if (seWindow.isKeyDown(GLFW_KEY_1)) {
				lightActor.getComponent<LightPropagationVolumeComponent>().cascadeCount = 1;
			}
			if (seWindow.isKeyDown(GLFW_KEY_2)) {
				lightActor.getComponent<LightPropagationVolumeComponent>().cascadeCount = 2;
			}
			if (seWindow.isKeyDown(GLFW_KEY_3)) {
				lightActor.getComponent<LightPropagationVolumeComponent>().cascadeCount = 3;
			}
			if (seWindow.isKeyDown(GLFW_KEY_4)) {
				lightActor.getComponent<LightPropagationVolumeComponent>().cascadeCount = 4;
			}
			if (seWindow.isKeyDown(GLFW_KEY_R)) {
				flPropCount -= frameTime;
				flPropCount = glm::max(flPropCount, 1.0f);
			}
			if (seWindow.isKeyDown(GLFW_KEY_T)) {
				flPropCount += frameTime;
			}
			bool previewSdf = seWindow.isKeyDown(GLFW_KEY_0);
			
			lightActor.getComponent<LightPropagationVolumeComponent>().propagationIterations = flPropCount;

			camera.setViewYXZ(cameraActor.getTransform().translation, cameraActor.getTransform().rotation);
			camera.setPerspectiveProjection(70.0f, seSwapChain->extentAspectRatio(), 0.01f, 128.f);

			teapotActor.getTransform().rotation.y = incrementTime;
			teapotActor.getTransform().scale = glm::vec3(1.0, 1.0, sin(incrementTime) + 2.0f);

			incrementTime += frameTime;
			//resourceSystem->getSurfaceMaterial(clearCoatMaterial)->roughnessFactor = (sin(incrementTime) + 1.0) * 0.5;
			//resourceSystem->getSurfaceMaterial(clearCoatMaterial)->updateParams();

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

				renderSystem->beginEarlyDepthPass(frameInfo);
				if (!previewSdf) {
					renderSystem->renderEarlyDepth(frameInfo);
				}
				renderSystem->endEarlyDepthPass(frameInfo);
				if (!previewSdf) {
					sdfRenderSystem->renderSdfShadows(frameInfo, lightActor);
				}
				renderSystem->beginOpaquePass(frameInfo);
				frameInfo.skyLight = skyLightSystem->getDescriptorSet(frameIndex);
				if (!previewSdf) {
					renderSystem->renderOpaque(frameInfo);
				} else {
					sdfPreviewer->renderSdfs(frameInfo);
				}
				if (pickedCube) {
					skyboxSystem->render(frameInfo, skyLightSystem->getDescriptorSet(frameIndex));
				}
				renderSystem->renderTranslucent(frameInfo);
				renderSystem->endOpaquePass(frameInfo);

				seSwapChain->beginRenderPass(commandBuffer);
				screenCorrection->render(frameInfo);
				SHUD::VulkanFrameInfo uiFrameInfo;
				uiFrameInfo.mCommandBuffer = commandBuffer;
				uiFrameInfo.mFrameIndex = frameIndex;
				uiContext->Render(&uiFrameInfo);
				seSwapChain->endRenderPass(commandBuffer);

				commandBuffers[frameIndex]->end();
				uiContext->Cleanup();

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
				sdfRenderSystem->resize({ extent.width, extent.height }, renderSystem->getDepthAttachment()->getDescriptor());
				renderSystem->setSdfShadowTexture(sdfRenderSystem->getShadowMaskDescriptor());
				screenCorrection->resize({ extent.width, extent.height }, { renderSystem->getColorAttachment() });
				uiContext->SetResolution({ (float)seWindow.getExtent().width, (float)seWindow.getExtent().height });
				seWindow.resetWindowResizedFlag();
			}
		}
		vkDeviceWaitIdle(seDevice.getDevice());

		MaterialSystem::destroyDescriptorSetLayout();
		delete renderSystem;
		delete screenCorrection;
		delete skyLightSystem;
		delete skyboxSystem;
		delete sdfPreviewer;
		//delete sdfRenderSystem;

		delete resourceSystem;
	}
}