#include "grass_app.h"

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
	GrassApp::GrassApp(const char* name) : VulkanBaseApp(name)
	{
	}
	GrassApp::~GrassApp()
	{
	}

	void GrassApp::run() {
		glm::vec2 resolution = { 1280, 720 };
		ResourceSystem* resourceSystem = new ResourceSystem(seDevice, *staticPool);
		MaterialSystem::createDescriptorSetLayout(seDevice);
		std::unique_ptr<SEDescriptorSetLayout> skyLightDescriptorLayout = SEDescriptorSetLayout::Builder(seDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // prefiltered
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // irradiance
			.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // brdfLUT
			.build();

		
		ShadowMapSystem* shadowMapSystem = new ShadowMapSystem(seDevice, *staticPool);

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
		ResourceID clearCoatMaterial = resourceSystem->loadSurfaceMaterial("res/materials/paint.json");
		ResourceID blankMaterial = resourceSystem->loadSurfaceMaterial("res/materials/blank.json");
		ResourceID glassMaterial = resourceSystem->loadSurfaceMaterial("res/materials/glass.json");
		ResourceID grassMaterial = resourceSystem->loadSurfaceMaterial("res/materials/grass.json");
		level = std::make_shared<Level>();
		Actor cameraActor = level->createActor("cameraActor");
		Actor lightActor = level->createActor("sun");
		Actor teapotActor = level->createActor("some kind of mesh");
		Actor grassActor0 = level->createActor("some kind of grass 0");
		Actor grassActor1 = level->createActor("some kind of grass 1");
		Actor grassActor2 = level->createActor("some kind of grass 2");
		{
			auto& grassMesh = grassActor0.addComponent<MeshComponent>();
			grassMesh.mesh = resourceSystem->loadModel("res/models/grass/grass_blades_0.fbx");
			for (const std::string& mapto : resourceSystem->getModel(grassMesh.mesh)->getSubmeshes()) {
				grassMesh.materials[mapto] = grassMaterial;
			}
		}
		{
			auto& grassMesh = grassActor1.addComponent<MeshComponent>();
			grassMesh.mesh = resourceSystem->loadModel("res/models/grass/grass_blades_1.fbx");
			for (const std::string& mapto : resourceSystem->getModel(grassMesh.mesh)->getSubmeshes()) {
				grassMesh.materials[mapto] = grassMaterial;
			}
		}
		{
			auto& grassMesh = grassActor2.addComponent<MeshComponent>();
			grassMesh.mesh = resourceSystem->loadModel("res/models/grass/grass_blades_2.fbx");
			for (const std::string& mapto : resourceSystem->getModel(grassMesh.mesh)->getSubmeshes()) {
				grassMesh.materials[mapto] = grassMaterial;
			}
		}
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

			{
				auto& teapotMesh = teapotActor.addComponent<MeshComponent>();
				teapotMesh.mesh = resourceSystem->loadModel("res/models/teapot.fbx");
				for (const std::string& mapto : resourceSystem->getModel(teapotMesh.mesh)->getSubmeshes()) {
					teapotMesh.materials[mapto] = glassMaterial;
				}
				teapotActor.getTransform().translation = { 4.f, 0.f, 3.0f };
				teapotActor.getTransform().rotation = { -1.51, 0.0, 0.0 };
				teapotActor.getTransform().scale = { 70.0, 70.0, 70.0 };
			}

			Actor skyboxActor = level->createActor("skyboxActor");
			auto& sbc = skyboxActor.addComponent<SkyboxComponent>();
			auto& slc = skyboxActor.addComponent<SkyLightComponent>();
			slc.environmentMap = resourceSystem->loadTextureCube("res/environmentmaps/skywater").id;
			slc.intensity = 0.80f;
			cameraActor.getTransform().translation = { 0.f, -1.f, 2.0f };
			{
				lightActor.addComponent<DirectionalLightComponent>();
				lightActor.addComponent<LightPropagationVolumeComponent>();
				lightActor.getTransform().rotation.x = 0.54f;
				lightActor.getTransform().rotation.z = 0.25f;
				lightActor.getComponent<DirectionalLightComponent>().intensity = 8.0f;
				lightActor.getComponent<DirectionalLightComponent>().shadow.maxDistance = 28.0f;
				lightActor.getComponent<LightPropagationVolumeComponent>().maxExtent = { 32, 32, 32 }; // coverage should be large for better results
				lightActor.getComponent<LightPropagationVolumeComponent>().cascadeCount = 0;
				lightActor.getComponent<LightPropagationVolumeComponent>().propagationIterations = 1;
			}
		}
		struct GrassPush {
			glm::vec4 minBounds;
			glm::vec4 maxBounds;
			glm::vec4 minScale;
			glm::vec4 maxScale;
			glm::vec4 windDirection;
			uint32_t seed;
			float elapsed;
		};
		SEPushConstant grassPush = SEPushConstant(sizeof(GrassPush), VK_SHADER_STAGE_VERTEX_BIT);
		SEPipelineLayout grassPipelineLayout = SEPipelineLayout(seDevice, { grassPush.getRange() }, {
			globalUBODescriptorLayout->getDescriptorSetLayout(),
			sceneDescriptorLayout->getDescriptorSetLayout(),
			skyLightDescriptorLayout->getDescriptorSetLayout(),
			shadowMapSystem->getDescriptorSetLayout(),
			MaterialSystem::getMaterialDescriptorSetLayout()->getDescriptorSetLayout()
		});
		SEGraphicsPipelineConfigInfo grassPipelineConfigInfo{};
		grassPipelineConfigInfo.enableVertexDescriptions();
		grassPipelineConfigInfo.setSampleCount(msaa);
		//grassPipelineConfigInfo.enableTesselation(4U);
		grassPipelineConfigInfo.renderPass = renderSystem->getOpaqueRenderPass()->getRenderPass();
		grassPipelineConfigInfo.pipelineLayout = grassPipelineLayout.getPipelineLayout();

		SEGraphicsPipeline grassPipeline = SEGraphicsPipeline(seDevice, grassPipelineConfigInfo, {
			SEShader(SEShaderType::Vertex, "res/shaders/spirv/grass_inst.vsh.spv"),
		//	SEShader(SEShaderType::TessControl, "res/shaders/spirv/grass_c.tsh.spv"),
		//	SEShader(SEShaderType::TessEval, "res/shaders/spirv/grass_e.tsh.spv"),
			SEShader(SEShaderType::Fragment, "res/shaders/spirv/forward_shading.fsh.spv")
		});

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
			
			lightActor.getComponent<LightPropagationVolumeComponent>().propagationIterations = flPropCount;

			camera.setViewYXZ(cameraActor.getTransform().translation, cameraActor.getTransform().rotation);
			camera.setPerspectiveProjection(70.0f, seSwapChain->extentAspectRatio(), 0.01f, 128.f);

			teapotActor.getTransform().translation.x = (sin(incrementTime) * 5.0);

			incrementTime += frameTime;
			resourceSystem->getSurfaceMaterial(clearCoatMaterial)->roughnessFactor = (sin(incrementTime) + 1.0) * 0.5;
			resourceSystem->getSurfaceMaterial(clearCoatMaterial)->updateParams();

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

				{ // grass 
					grassPipeline.bind(commandBuffer);
					VkDescriptorSet sets[4]{
						frameInfo.globalUBO,
						frameInfo.sceneSSBO,
						frameInfo.skyLight,
						frameInfo.shadowMap
					};
					vkCmdBindDescriptorSets(
						commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						grassPipelineLayout.getPipelineLayout(),
						0,
						4,
						sets,
						0,
						nullptr
					);
					SESurfaceMaterial* sfMaterial = resourceSystem->getSurfaceMaterial(grassMaterial);
					sfMaterial->bind(commandBuffer, grassPipelineLayout.getPipelineLayout(), 4);
					GrassPush push;
					const float grassext = 30.0f;
					push.minBounds = { -grassext, -0.2f, -grassext, 0.0f };
					push.maxBounds = { grassext, -0.05f, grassext, 0.0f };
					push.minScale = { 0.8f, 0.8f, 0.8f, 0.0f };
					push.maxScale = { 1.8f, 2.5f, 1.8f, 0.0f };
					push.seed = 5623189510u;
					push.elapsed = incrementTime;
					push.windDirection = glm::normalize(glm::vec4{ 1.0, -0.05, 0.7f, 0.0f });
					std::vector<Actor> grassActors{ grassActor0, grassActor1, grassActor2 };
					for (auto& grassActor : grassActors) {
						for (auto& [slot, material] : grassActor.getComponent<MeshComponent>().materials) {
							push.seed += 8185295122948150912u;
							grassPush.push(commandBuffer, grassPipelineLayout.getPipelineLayout(), &push);
							SEModel* model = resourceSystem->getModel(grassActor.getComponent<MeshComponent>().mesh);
							model->bind(commandBuffer, slot);
							model->draw(commandBuffer, 10000);
						}
					}

				} // end grass
				

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