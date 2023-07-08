#include "shadow_map_system.h"

#include <scorch/ecs/components.h>
#include <scorch/ecs/actor.h>
#include <scorch/systems/resource_system.h>
#include <glm/gtx/rotate_vector.hpp>

namespace ScorchEngine {
	struct RSMPush {
		glm::mat4 vp;
		glm::mat4 modelMatrix;
	};

	struct LPVInjectData {
		glm::mat4 rsmInvProj;
		glm::mat4 rsmInvView;
		glm::mat4 rsmVP;
		alignas(16)glm::vec3 lpvExtent;
		alignas(16)glm::vec3 lpvCenter;
		alignas(16)glm::vec3 lpvCellSize;
		alignas(16)glm::vec3 lightDirection;
		alignas(16)glm::vec3 lightIntensity;
	};

	ShadowMapSystem::ShadowMapSystem(
		SEDevice& device, SEDescriptorPool& descriptorPool) : seDevice(device), seDescriptorPool(descriptorPool) {
		init();
		createGraphicsPipelines();
	}
	ShadowMapSystem::~ShadowMapSystem() {
		destroy();
		delete shadowMapPipeline;
		delete shadowMapPipelineLayout;
	}

	void ShadowMapSystem::init() {
		createFramebufferAttachments();
		createRenderPasses();
		createFramebuffers();
		createLPV();
		writeDescriptor();
	}

	void ShadowMapSystem::destroy() {
		delete shadowMapAttachment;
		delete shadowMapFramebuffer;
		delete shadowMapRenderPass;
		delete rsmDepthAttachment;
		delete rsmNormalAttachment;
		delete rsmFluxAttachment;
		delete rsmFramebuffer;
		delete rsmRenderPass;
		delete lpvRedSH;
		delete lpvGreenSH;
		delete lpvBlueSH;

		delete shadowMapPipelineLayout;
		delete shadowMapPipeline;
		delete rsmPipelineLayout;
		delete rsmPipeline;

		delete lpvPipelineLayout;
		delete lpvComputeInjection;
		delete lpvComputePropagation;
	}

	void ShadowMapSystem::createLPV() {
		SEVoxelTextureCreateInfo vx{};
		vx.dimensions = { LPV_RESOLUTION ,LPV_RESOLUTION,LPV_RESOLUTION };
		vx.layout = VK_IMAGE_LAYOUT_GENERAL;
		vx.linearFiltering = true;
		vx.voxelFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		vx.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		lpvRedSH = new SEVoxelTexture(seDevice, vx);
		lpvGreenSH = new SEVoxelTexture(seDevice, vx);
		lpvBlueSH = new SEVoxelTexture(seDevice, vx);
		lpvPropRedSH = new SEVoxelTexture(seDevice, vx);
		lpvPropGreenSH = new SEVoxelTexture(seDevice, vx);
		lpvPropBlueSH = new SEVoxelTexture(seDevice, vx);

		VkCommandBuffer cb = seDevice.beginSingleTimeCommands();
		seDevice.transitionImageLayout(cb, lpvRedSH->getImage(), vx.voxelFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1, 1);
		seDevice.transitionImageLayout(cb, lpvGreenSH->getImage(), vx.voxelFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1, 1);
		seDevice.transitionImageLayout(cb, lpvBlueSH->getImage(), vx.voxelFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1, 1);
		seDevice.transitionImageLayout(cb, lpvPropRedSH->getImage(), vx.voxelFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1, 1);
		seDevice.transitionImageLayout(cb, lpvPropGreenSH->getImage(), vx.voxelFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1, 1);
		seDevice.transitionImageLayout(cb, lpvPropBlueSH->getImage(), vx.voxelFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1, 1);
		seDevice.endSingleTimeCommands(cb);

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			lpvInjectionData[i] = std::make_unique<SEBuffer>(
				seDevice,
				sizeof(LPVInjectData),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
				);
			lpvInjectionData[i]->map();
		}
	}

	void ShadowMapSystem::writeDescriptor() {
		shadowMapDescriptorSetLayout = SEDescriptorSetLayout::Builder(seDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();
		auto shadowMapImageInfo = shadowMapAttachment->getDescriptor();
		auto lpvPropRedSHImageInfo = lpvPropRedSH->getDescriptor();
		auto lpvPropGreenSHImageInfo = lpvPropGreenSH->getDescriptor();
		auto lpvPropBlueSHImageInfo = lpvPropBlueSH->getDescriptor();
		auto lpvRedSHImageInfo = lpvRedSH->getDescriptor();
		auto lpvGreenSHImageInfo = lpvGreenSH->getDescriptor();
		auto lpvBlueSHImageInfo = lpvBlueSH->getDescriptor();
		SEDescriptorWriter(*shadowMapDescriptorSetLayout, seDescriptorPool)
			.writeImage(0, &shadowMapImageInfo)
			.writeImage(1, &lpvPropRedSHImageInfo)
			.writeImage(2, &lpvPropGreenSHImageInfo)
			.writeImage(3, &lpvPropBlueSHImageInfo)
			.build(shadowMapDescriptorSet);

		lpvGenerationDataDescriptorSetLayout = SEDescriptorSetLayout::Builder(seDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(4, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(5, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(6, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(7, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(8, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(9, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		// remove the samplers since we are using this as a storage image for the following descriptor
		lpvRedSHImageInfo.sampler = VK_NULL_HANDLE;
		lpvGreenSHImageInfo.sampler = VK_NULL_HANDLE;
		lpvBlueSHImageInfo.sampler = VK_NULL_HANDLE;
		lpvPropRedSHImageInfo.sampler = VK_NULL_HANDLE;
		lpvPropGreenSHImageInfo.sampler = VK_NULL_HANDLE;
		lpvPropBlueSHImageInfo.sampler = VK_NULL_HANDLE;

		auto rsmDepthData = rsmDepthAttachment->getDescriptor();
		auto rsmNormalData = rsmNormalAttachment->getDescriptor();
		auto rsmFluxData = rsmFluxAttachment->getDescriptor();

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			auto injData = lpvInjectionData[i]->getDescriptorInfo();
			SEDescriptorWriter(*lpvGenerationDataDescriptorSetLayout, seDescriptorPool)
				.writeBuffer(0, &injData)
				.writeImage(1, &rsmDepthData)
				.writeImage(2, &rsmNormalData)
				.writeImage(3, &rsmFluxData)
				.writeImage(4, &lpvRedSHImageInfo)
				.writeImage(5, &lpvGreenSHImageInfo)
				.writeImage(6, &lpvBlueSHImageInfo)
				.writeImage(7, &lpvPropRedSHImageInfo)
				.writeImage(8, &lpvPropGreenSHImageInfo)
				.writeImage(9, &lpvPropBlueSHImageInfo)
				.build(lpvGenerationDataDescriptorSet[i]);
		}
	}

	void ShadowMapSystem::render(FrameInfo& frameInfo) {

		DirectionalLightComponent* light;
		LightPropagationVolumeComponent* lpv;
		TransformComponent* transform;

		// iterate over candidates
		for (auto& a : frameInfo.level->getRegistry().view<DirectionalLightComponent, LightPropagationVolumeComponent, TransformComponent>()) {
			Actor actor = { a, frameInfo.level.get() };

			light = &actor.getComponent<DirectionalLightComponent>();
			lpv = &actor.getComponent<LightPropagationVolumeComponent>();
			transform = &actor.getTransform();
		}

		float clipBounds = light->shadow.maxDistance;
		glm::mat4 realTransform = transform->getTransformMatrix();
		realTransform = glm::rotate(realTransform, glm::half_pi<float>(), glm::vec3(1.f, 0.f, 0.f));
		realTransform[3] = glm::vec4(frameInfo.camera.getPosition(), 1.0f);

		SECamera shadowMapCamera;
		shadowMapCamera.setViewYXZ(realTransform);
		shadowMapCamera.setOrthographicProjection(
			-clipBounds,
			clipBounds,
			-clipBounds,
			clipBounds,
			-220.000f, 80.000f
		);

		light->shadow.vp = shadowMapCamera.getProjection() * shadowMapCamera.getView();


		{
			// avoid jaggedness on static objects when moving the camera by snapping the position to the nearest original pixel
			float halfRes = static_cast<float>(SHADOW_MAP_RESOLUTION) / 2.0f;
		
			// pick random point, doesn't matter
			glm::vec4 originPoint = glm::vec4(0.0, 0.0, 0.0, 1.0);
			glm::vec4 clip = light->shadow.vp * originPoint;
		
			clip /= clip.w;
			glm::vec2 full = glm::floor(clip * halfRes);
			glm::vec4 newClip = shadowMapCamera.getInverseProjection() * glm::vec4(full / halfRes, clip.z, 1.0);
			newClip /= newClip.w;
			glm::vec4 newWorld = shadowMapCamera.getInverseView() * newClip;
			glm::vec4 difference = originPoint - newWorld;
			realTransform[3] += difference;
			realTransform[3].w = 1.0f;
			shadowMapCamera.setViewYXZ(realTransform);
			light->shadow.vp = shadowMapCamera.getProjection() * shadowMapCamera.getView();
		}

		// regular shadow map pass
		shadowMapRenderPass->beginRenderPass(frameInfo.commandBuffer, shadowMapFramebuffer);
		shadowMapPipeline->bind(frameInfo.commandBuffer);

		frameInfo.level->getRegistry().view<Components::TransformComponent, Components::MeshComponent>().each(
			[&](auto& tfc, auto& msc) {
				SEModel* model = frameInfo.resourceSystem->getModel(msc.mesh);
				glm::mat4 mpv = light->shadow.vp * tfc.getTransformMatrix();
				shadowPush.push(frameInfo.commandBuffer, shadowMapPipelineLayout->getPipelineLayout(), &mpv);

				for (const auto& [mapTo, matAsset] : msc.materials) {
					SESurfaceMaterial* material = frameInfo.resourceSystem->getSurfaceMaterial(matAsset);
					if (material->translucent) continue;
					material->bind(frameInfo.commandBuffer, shadowMapPipelineLayout->getPipelineLayout(), 0);
					model->bind(frameInfo.commandBuffer, mapTo);
					model->draw(frameInfo.commandBuffer);
				}
			}
		);

		shadowMapRenderPass->endRenderPass(frameInfo.commandBuffer);

		glm::mat4 vpRSM;
		{
			// avoid jaggedness on static objects when moving the camera by snapping the position to the nearest original pixel
			float halfRes = static_cast<float>(SHADOW_MAP_RSM_RESOLUTION) / 2.0f;

			// pick random point, doesn't matter
			glm::vec4 originPoint = glm::vec4(0.0, 0.0, 0.0, 1.0);
			glm::vec4 clip = light->shadow.vp * originPoint;

			clip /= clip.w;
			glm::vec2 full = glm::floor(clip * halfRes);
			glm::vec4 newClip = shadowMapCamera.getInverseProjection() * glm::vec4(full / halfRes, clip.z, 1.0);
			newClip /= newClip.w;
			glm::vec4 newWorld = shadowMapCamera.getInverseView() * newClip;
			glm::vec4 difference = originPoint - newWorld;
			realTransform[3] += difference;
			realTransform[3].w = 1.0f;
			shadowMapCamera.setViewYXZ(realTransform);
			vpRSM = shadowMapCamera.getProjection() * shadowMapCamera.getView();
		}
		// RSM pass
		rsmRenderPass->beginRenderPass(frameInfo.commandBuffer, rsmFramebuffer);
		rsmPipeline->bind(frameInfo.commandBuffer);

		frameInfo.level->getRegistry().view<Components::TransformComponent, Components::MeshComponent>().each(
			[&](auto& tfc, auto& msc) {
				SEModel* model = frameInfo.resourceSystem->getModel(msc.mesh);
				RSMPush push{};
				push.modelMatrix = tfc.getTransformMatrix();
				push.vp = vpRSM;

				rsmPush.push(frameInfo.commandBuffer, rsmPipelineLayout->getPipelineLayout(), &push);

				for (const auto& [mapTo, matAsset] : msc.materials) {
					SESurfaceMaterial* material = frameInfo.resourceSystem->getSurfaceMaterial(matAsset);
					if (material->translucent) continue;
					material->bind(frameInfo.commandBuffer, rsmPipelineLayout->getPipelineLayout(), 0);
					model->bind(frameInfo.commandBuffer, mapTo);
					model->draw(frameInfo.commandBuffer);
				}
			}
		);

		rsmRenderPass->endRenderPass(frameInfo.commandBuffer);

		LPVInjectData lpvInject{};
		lpvInject.lightDirection = glm::normalize(realTransform[2]);
		lpvInject.lightIntensity = light->intensity * light->emission;
		lpvInject.lpvCellSize = lpv->extent / glm::vec3(LPV_RESOLUTION);
		lpvInject.lpvCenter = realTransform[3];
		lpvInject.lpvExtent = lpv->extent;
		lpvInject.rsmVP = vpRSM;
		lpvInject.rsmInvProj = shadowMapCamera.getInverseProjection();
		lpvInject.rsmInvView = shadowMapCamera.getInverseView();
		{
			// avoid jaggedness on static objects when moving the camera by snapping the position to the nearest original pixel
			glm::vec3 halfRes = glm::vec3(LPV_RESOLUTION) / 2.0f;

			// pick random point, doesn't matter
			glm::vec3 originPoint = glm::vec4(0.0, 0.0, 0.0, 1.0);
			glm::vec3 uv = (originPoint - lpvInject.lpvCenter) / lpvInject.lpvExtent;
			glm::vec3 full = floor(uv * halfRes);

			glm::vec3 newWorld = full / lpvInject.lpvExtent + lpvInject.lpvCenter;
			glm::vec3 difference = originPoint - newWorld;
			lpvInject.lpvCenter += difference;
		}
		lpv->center = lpvInject.lpvCenter;

		lpvInjectionData[frameInfo.frameIndex]->writeToBuffer(&lpvInject);
		lpvInjectionData[frameInfo.frameIndex]->flush();
		// LPV passes

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_COMPUTE,
			lpvPipelineLayout->getPipelineLayout(),
			0,
			1,
			&lpvGenerationDataDescriptorSet[frameInfo.frameIndex],
			0,
			nullptr
		);

		lpvComputeClear->bind(frameInfo.commandBuffer);
		vkCmdDispatch(frameInfo.commandBuffer, LPV_RESOLUTION / 16U, LPV_RESOLUTION / 16U, LPV_RESOLUTION);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			lpvComputeInjection->getPipelineLayout(),
			0,
			1,
			&lpvGenerationDataDescriptorSet[frameInfo.frameIndex],
			0,
			nullptr
		);
		lpvComputeInjection->render(frameInfo.commandBuffer, nullptr);
		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			lpvComputePropagation->getPipelineLayout(),
			0,
			1,
			&lpvGenerationDataDescriptorSet[frameInfo.frameIndex],
			0,
			nullptr
		);
		for (int i = 0; i < LPV_PROPGATION_FASES; i++) {
			lpvComputePropagation->render(frameInfo.commandBuffer, nullptr);
		}
	}


	void ShadowMapSystem::createFramebufferAttachments() {
		SEFramebufferAttachmentCreateInfo shadowAttachmentCreateInfo{};
		shadowAttachmentCreateInfo.dimensions = glm::ivec3(SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, 1);
		shadowAttachmentCreateInfo.framebufferFormat = VK_FORMAT_D16_UNORM;
		shadowAttachmentCreateInfo.framebufferType = SEFramebufferAttachmentType::Depth;
		shadowAttachmentCreateInfo.imageAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		shadowAttachmentCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		shadowAttachmentCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		shadowAttachmentCreateInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		shadowAttachmentCreateInfo.sampleCount = VK_SAMPLE_COUNT_1_BIT;
		shadowAttachmentCreateInfo.linearFiltering = true;
		shadowAttachmentCreateInfo.isShadowMap = true;
		shadowMapAttachment = new SEFramebufferAttachment(seDevice, shadowAttachmentCreateInfo);


		SEFramebufferAttachmentCreateInfo rsmDepthAttachmentCreateInfo{};
		rsmDepthAttachmentCreateInfo.dimensions = glm::ivec3(SHADOW_MAP_RSM_RESOLUTION, SHADOW_MAP_RSM_RESOLUTION, 1);
		rsmDepthAttachmentCreateInfo.framebufferFormat = VK_FORMAT_D16_UNORM;
		rsmDepthAttachmentCreateInfo.framebufferType = SEFramebufferAttachmentType::Depth;
		rsmDepthAttachmentCreateInfo.imageAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		rsmDepthAttachmentCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		rsmDepthAttachmentCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		rsmDepthAttachmentCreateInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		rsmDepthAttachmentCreateInfo.sampleCount = VK_SAMPLE_COUNT_1_BIT;
		rsmDepthAttachmentCreateInfo.linearFiltering = false;
		rsmDepthAttachmentCreateInfo.isShadowMap = false;
		rsmDepthAttachment = new SEFramebufferAttachment(seDevice, rsmDepthAttachmentCreateInfo);

		SEFramebufferAttachmentCreateInfo rsmNormalAttachmentCreateInfo{};
		rsmNormalAttachmentCreateInfo.dimensions = glm::ivec3(SHADOW_MAP_RSM_RESOLUTION, SHADOW_MAP_RSM_RESOLUTION, 1);
		rsmNormalAttachmentCreateInfo.framebufferFormat = VK_FORMAT_R16G16B16A16_SNORM;
		rsmNormalAttachmentCreateInfo.framebufferType = SEFramebufferAttachmentType::Color;
		rsmNormalAttachmentCreateInfo.imageAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		rsmNormalAttachmentCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		rsmNormalAttachmentCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		rsmNormalAttachmentCreateInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		rsmNormalAttachmentCreateInfo.sampleCount = VK_SAMPLE_COUNT_1_BIT;
		rsmNormalAttachmentCreateInfo.linearFiltering = false;
		rsmNormalAttachmentCreateInfo.isShadowMap = false;
		rsmNormalAttachment = new SEFramebufferAttachment(seDevice, rsmNormalAttachmentCreateInfo);

		SEFramebufferAttachmentCreateInfo rsmFluxAttachmentCreateInfo{};
		rsmFluxAttachmentCreateInfo.dimensions = glm::ivec3(SHADOW_MAP_RSM_RESOLUTION, SHADOW_MAP_RSM_RESOLUTION, 1);
		rsmFluxAttachmentCreateInfo.framebufferFormat = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		rsmFluxAttachmentCreateInfo.framebufferType = SEFramebufferAttachmentType::Color;
		rsmFluxAttachmentCreateInfo.imageAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		rsmFluxAttachmentCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		rsmFluxAttachmentCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		rsmFluxAttachmentCreateInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		rsmFluxAttachmentCreateInfo.sampleCount = VK_SAMPLE_COUNT_1_BIT;
		rsmFluxAttachmentCreateInfo.linearFiltering = true;
		rsmFluxAttachmentCreateInfo.isShadowMap = false;
		rsmFluxAttachment = new SEFramebufferAttachment(seDevice, rsmFluxAttachmentCreateInfo);
	}

	void ShadowMapSystem::createRenderPasses() {
		SEAttachmentInfo shadowAttachmentInfo{};
		shadowAttachmentInfo.framebufferAttachment = shadowMapAttachment;
		shadowAttachmentInfo.clear.depth = { 1.0, 0 };
		shadowAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		shadowAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		shadowMapRenderPass = new SERenderPass(seDevice, { shadowAttachmentInfo });

		SEAttachmentInfo rsmDepthAttachmentInfo{};
		rsmDepthAttachmentInfo.framebufferAttachment = rsmDepthAttachment;
		rsmDepthAttachmentInfo.clear.depth = { 1.0, 0 };
		rsmDepthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		rsmDepthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		SEAttachmentInfo rsmNormalAttachmentInfo{};
		rsmNormalAttachmentInfo.framebufferAttachment = rsmNormalAttachment;
		rsmNormalAttachmentInfo.clear.color = { 0.0, 0.0, 0.0, 1.0 };
		rsmNormalAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		rsmNormalAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		SEAttachmentInfo rsmFluxAttachmentInfo{};
		rsmFluxAttachmentInfo.framebufferAttachment = rsmFluxAttachment;
		rsmFluxAttachmentInfo.clear.color = { 0.0, 0.0, 0.0, 1.0 };
		rsmFluxAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		rsmFluxAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		rsmRenderPass = new SERenderPass(seDevice, { rsmDepthAttachmentInfo, rsmNormalAttachmentInfo,  rsmFluxAttachmentInfo });
	}

	void ShadowMapSystem::createFramebuffers() {
		shadowMapFramebuffer = new SEFramebuffer(seDevice, shadowMapRenderPass, { shadowMapAttachment });
		rsmFramebuffer = new SEFramebuffer(seDevice, rsmRenderPass, { rsmDepthAttachment, rsmNormalAttachment, rsmFluxAttachment });
	}

	void ShadowMapSystem::createGraphicsPipelines() {
		shadowPush = SEPushConstant(sizeof(glm::mat4), VK_SHADER_STAGE_VERTEX_BIT);

		shadowMapPipelineLayout = new SEPipelineLayout(seDevice, { shadowPush.getRange() }, { MaterialSystem::getMaterialDescriptorSetLayout()->getDescriptorSetLayout() });

		SEGraphicsPipelineConfigInfo shadowPipelineConfigInfo{};
		shadowPipelineConfigInfo.enableVertexDescriptions();
		// remove normals and tangents, we dont need them
		shadowPipelineConfigInfo.attributeDescriptions.pop_back();
		shadowPipelineConfigInfo.attributeDescriptions.pop_back();
		shadowPipelineConfigInfo.bindingDescriptions.pop_back();
		shadowPipelineConfigInfo.bindingDescriptions.pop_back();

		shadowPipelineConfigInfo.renderPass = shadowMapRenderPass->getRenderPass();
		shadowPipelineConfigInfo.pipelineLayout = shadowMapPipelineLayout->getPipelineLayout();
		shadowMapPipeline = new SEGraphicsPipeline(
			seDevice,
			shadowPipelineConfigInfo,
			{ SEShader(SEShaderType::Vertex, "res/shaders/spirv/shadow.vsh.spv"), SEShader(SEShaderType::Fragment, "res/shaders/spirv/shadow.fsh.spv") }
		);

		rsmPush = SEPushConstant(sizeof(RSMPush), VK_SHADER_STAGE_VERTEX_BIT);

		rsmPipelineLayout = new SEPipelineLayout(seDevice, { rsmPush.getRange() }, { MaterialSystem::getMaterialDescriptorSetLayout()->getDescriptorSetLayout() });

		SEGraphicsPipelineConfigInfo rsmPipelineConfigInfo{2};
		rsmPipelineConfigInfo.enableVertexDescriptions();

		rsmPipelineConfigInfo.renderPass = rsmRenderPass->getRenderPass();
		rsmPipelineConfigInfo.pipelineLayout = rsmPipelineLayout->getPipelineLayout();
		rsmPipeline = new SEGraphicsPipeline(
			seDevice,
			rsmPipelineConfigInfo,
			{ SEShader(SEShaderType::Vertex, "res/shaders/spirv/shadow_rsm.vsh.spv"), SEShader(SEShaderType::Fragment, "res/shaders/spirv/shadow_rsm.fsh.spv") }
		);


		lpvPipelineLayout = new SEPipelineLayout(seDevice, { }, { lpvGenerationDataDescriptorSetLayout->getDescriptorSetLayout() });
		lpvComputeClear = new SEComputePipeline(
			seDevice,
			lpvPipelineLayout->getPipelineLayout(),
			{ SEShader(SEShaderType::Compute, "res/shaders/spirv/lpv_clear.csh.spv")}
		);
		lpvComputeInjection = new SEPostProcessingEffect(
			seDevice,
			{SHADOW_MAP_RSM_RESOLUTION, SHADOW_MAP_RSM_RESOLUTION},
			SEShader(SEShaderType::Fragment, "res/shaders/spirv/lpv_injection.fsh.spv"),
			seDescriptorPool,
			{},
			VK_FORMAT_R8_UINT,
			VK_IMAGE_VIEW_TYPE_2D,
			{ lpvGenerationDataDescriptorSetLayout->getDescriptorSetLayout() }
		);
		lpvComputePropagation = new SEPostProcessingEffect(
			seDevice,
			{ LPV_RESOLUTION, LPV_RESOLUTION },
			SEShader(SEShaderType::Fragment, "res/shaders/spirv/lpv_propagation.fsh.spv"),
			seDescriptorPool,
			{},
			VK_FORMAT_R8_UINT,
			VK_IMAGE_VIEW_TYPE_2D,
			{ lpvGenerationDataDescriptorSetLayout->getDescriptorSetLayout() }
		);
	}
}