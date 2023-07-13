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
		alignas(16)glm::vec3 lpvCenter;
		alignas(16)glm::vec3 lightDirection;
		alignas(16)glm::vec3 lightIntensity;
	};

	struct LPVPush {
		alignas(16)glm::vec3 lpvExtent;
		alignas(16)glm::vec3 lpvCellSize;
		alignas(16)glm::ivec3 virtualPropagatedGridRedCoords;
		alignas(16)glm::ivec3 virtualPropagatedGridGreenCoords;
		alignas(16)glm::ivec3 virtualPropagatedGridBlueCoords;
		int pingPongIndex;
	};

	struct LPVMix {
		alignas(16)glm::ivec3 virtualPropagatedGridRedCoords;
		alignas(16)glm::ivec3 virtualPropagatedGridGreenCoords;
		alignas(16)glm::ivec3 virtualPropagatedGridBlueCoords;
		int pingPongIndex;
		float temporalBlend;
	};

	static glm::ivec3 virtualGridCoordinates[4][3]{
		{ // Cascade 0
			{0, 0, 0}, {LPV_RESOLUTION, 0, 0}, {2 * LPV_RESOLUTION, 0, 0}, // LPV R, G, B
		},
		{ // Cascade 1
			{ 0, LPV_RESOLUTION, 0 }, {LPV_RESOLUTION, LPV_RESOLUTION, 0}, {2 * LPV_RESOLUTION, LPV_RESOLUTION, 0}, // LPV R, G, B
		},
		{ // Cascade 2
			{ 0, 2 * LPV_RESOLUTION, 0 }, {LPV_RESOLUTION, 2 * LPV_RESOLUTION, 0}, {2 * LPV_RESOLUTION, 2 * LPV_RESOLUTION, 0} // LPV R, G, B
		},
		{ // Cascade 3
			{ 0, 3 * LPV_RESOLUTION, 0 }, {LPV_RESOLUTION, 3 * LPV_RESOLUTION, 0}, {2 * LPV_RESOLUTION, 3 * LPV_RESOLUTION, 0} // LPV R, G, B
		}
	};

	ShadowMapSystem::ShadowMapSystem(
		SEDevice& device, SEDescriptorPool& descriptorPool) : seDevice(device), seDescriptorPool(descriptorPool) {
		init();
	}
	ShadowMapSystem::~ShadowMapSystem() {
		destroy();
	}

	void ShadowMapSystem::init() {
		createFramebufferAttachments();
		createRenderPasses();
		createFramebuffers();
		createLPV();
		createDescriptorSetLayouts();
		createGraphicsPipelines();
		writeDescriptor();
	}

	void ShadowMapSystem::destroy() {
		delete shadowMapAttachment;
		delete shadowMapFramebuffer;
		delete shadowMapRenderPass;
		delete vfaoMapDepthAttachment;
		delete vfaoMapVarianceAttachment;
		delete vfaoMapFramebuffer;
		delete vfaoMapRenderPass;
		delete rsmDepthAttachment;
		delete rsmNormalAttachment;
		delete rsmFluxAttachment;
		delete rsmFramebuffer;
		delete rsmRenderPass;
		delete lpvInoutRedSH;
		delete lpvInoutGreenSH;
		delete lpvInoutBlueSH;
		delete lpvInout2RedSH;
		delete lpvInout2GreenSH;
		delete lpvInout2BlueSH;
		delete lpvPropagatedAtlasSH;

		delete shadowMapPipelineLayout;
		delete shadowMapPipeline;
		delete vfaoMapPipelineLayout;
		delete vfaoMapPipeline;
		delete rsmPipelineLayout;
		delete rsmPipeline;

		delete lpvComputeClearPipelineLayout;
		delete lpvComputeTemporalBlendPipelineLayout;
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
		lpvInoutRedSH = new SEVoxelTexture(seDevice, vx);
		lpvInoutGreenSH = new SEVoxelTexture(seDevice, vx);
		lpvInoutBlueSH = new SEVoxelTexture(seDevice, vx);
		lpvInout2RedSH = new SEVoxelTexture(seDevice, vx);
		lpvInout2GreenSH = new SEVoxelTexture(seDevice, vx);
		lpvInout2BlueSH = new SEVoxelTexture(seDevice, vx);
		vx.dimensions = { VIRTUAL_VOXEL_ATLAS_SIZE ,VIRTUAL_VOXEL_ATLAS_SIZE,VIRTUAL_VOXEL_ATLAS_SIZE };
		lpvPropagatedAtlasSH = new SEVoxelTexture(seDevice, vx);

		VkCommandBuffer cb = seDevice.beginSingleTimeCommands();
		seDevice.transitionImageLayout(cb, lpvInoutRedSH->getImage(), vx.voxelFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1, 1);
		seDevice.transitionImageLayout(cb, lpvInoutGreenSH->getImage(), vx.voxelFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1, 1);
		seDevice.transitionImageLayout(cb, lpvInoutBlueSH->getImage(), vx.voxelFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1, 1);
		seDevice.transitionImageLayout(cb, lpvInout2RedSH->getImage(), vx.voxelFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1, 1);
		seDevice.transitionImageLayout(cb, lpvInout2GreenSH->getImage(), vx.voxelFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1, 1);
		seDevice.transitionImageLayout(cb, lpvInout2BlueSH->getImage(), vx.voxelFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1, 1);
		seDevice.transitionImageLayout(cb, lpvPropagatedAtlasSH->getImage(), vx.voxelFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1, 1);
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
		auto shadowMapImageInfo = shadowMapAttachment->getDescriptor();
		auto lpvPropAtlasSHImageInfo = lpvPropagatedAtlasSH->getDescriptor();
		auto vfaoMapImageInfo = vfaoBlurFieldsV->getAttachment()->getDescriptor();
		SEDescriptorWriter(*shadowMapDescriptorSetLayout, seDescriptorPool)
			.writeImage(0, &shadowMapImageInfo)
			.writeImage(1, &lpvPropAtlasSHImageInfo)
			.writeImage(2, &vfaoMapImageInfo)
			.build(shadowMapDescriptorSet);

	
		auto lpvInoutRedSHImageInfo = lpvInoutRedSH->getDescriptor();
		auto lpvInoutGreenSHImageInfo = lpvInoutGreenSH->getDescriptor();
		auto lpvInoutBlueSHImageInfo = lpvInoutBlueSH->getDescriptor();
		auto lpvInout2RedSHImageInfo = lpvInout2RedSH->getDescriptor();
		auto lpvInout2GreenSHImageInfo = lpvInout2GreenSH->getDescriptor();
		auto lpvInout2BlueSHImageInfo = lpvInout2BlueSH->getDescriptor();
		// remove the samplers since we are using this as a storage image for the following descriptor
		lpvInoutRedSHImageInfo.sampler = VK_NULL_HANDLE;
		lpvInoutGreenSHImageInfo.sampler = VK_NULL_HANDLE;
		lpvInoutBlueSHImageInfo.sampler = VK_NULL_HANDLE;
		lpvInout2RedSHImageInfo.sampler = VK_NULL_HANDLE;
		lpvInout2GreenSHImageInfo.sampler = VK_NULL_HANDLE;
		lpvInout2BlueSHImageInfo.sampler = VK_NULL_HANDLE;
		lpvPropAtlasSHImageInfo.sampler = VK_NULL_HANDLE;

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
				.writeImage(4, &lpvInoutRedSHImageInfo)
				.writeImage(5, &lpvInoutGreenSHImageInfo)
				.writeImage(6, &lpvInoutBlueSHImageInfo)
				.writeImage(7, &lpvInout2RedSHImageInfo)
				.writeImage(8, &lpvInout2GreenSHImageInfo)
				.writeImage(9, &lpvInout2BlueSHImageInfo)
				.writeImage(10, &lpvPropAtlasSHImageInfo)
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
		lpvInject.lpvCenter = realTransform[3];
		lpvInject.rsmVP = vpRSM;
		lpvInject.rsmInvProj = shadowMapCamera.getInverseProjection();
		lpvInject.rsmInvView = shadowMapCamera.getInverseView();
		{
			// avoid jaggedness on static objects when moving the camera by snapping the position to the nearest original pixel
			glm::vec3 halfRes = glm::vec3(LPV_RESOLUTION) / 2.0f;

			// pick random point, doesn't matter
			glm::vec3 originPoint = glm::vec4(0.0, 0.0, 0.0, 1.0);
			glm::vec3 uv = (originPoint - lpvInject.lpvCenter) / lpv->maxExtent;
			glm::vec3 full = floor(uv * halfRes);

			glm::vec3 newWorld = full / lpv->maxExtent + lpvInject.lpvCenter;
			glm::vec3 difference = originPoint - newWorld;
			lpvInject.lpvCenter += difference;
		}
		lpv->center = lpvInject.lpvCenter;
		lpvInjectionData[frameInfo.frameIndex]->writeToBuffer(&lpvInject);
		lpvInjectionData[frameInfo.frameIndex]->flush();


		for (int i = 0; i < lpv->cascadeCount; i++) {
			// LPV passes

			vkCmdBindDescriptorSets(
				frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_COMPUTE,
				lpvComputeClearPipelineLayout->getPipelineLayout(),
				0,
				1,
				&lpvGenerationDataDescriptorSet[frameInfo.frameIndex],
				0,
				nullptr
			);

			lpvComputeClear->bind(frameInfo.commandBuffer);
			vkCmdDispatch(frameInfo.commandBuffer, LPV_RESOLUTION / 16U, LPV_RESOLUTION / 16U, LPV_RESOLUTION);

			glm::vec3 cascadeExtent = lpv->maxExtent / powf(2, lpv->cascadeCount - i - 1);
			LPVPush push;
			push.lpvCellSize = cascadeExtent / glm::vec3(LPV_RESOLUTION);
			push.lpvExtent = cascadeExtent;
			push.virtualPropagatedGridRedCoords = virtualGridCoordinates[i][0];
			push.virtualPropagatedGridGreenCoords = virtualGridCoordinates[i][1];
			push.virtualPropagatedGridBlueCoords = virtualGridCoordinates[i][2];

			lpv->cascades[i].virtualPropagatedGridRedUVMin = glm::vec3(virtualGridCoordinates[i][0]) / glm::vec3(VIRTUAL_VOXEL_ATLAS_SIZE);
			lpv->cascades[i].virtualPropagatedGridRedUVMax = glm::vec3(virtualGridCoordinates[i][0] + glm::ivec3(LPV_RESOLUTION)) / glm::vec3(VIRTUAL_VOXEL_ATLAS_SIZE);
			lpv->cascades[i].virtualPropagatedGridGreenUVMin = glm::vec3(virtualGridCoordinates[i][1]) / glm::vec3(VIRTUAL_VOXEL_ATLAS_SIZE);
			lpv->cascades[i].virtualPropagatedGridGreenUVMax = glm::vec3(virtualGridCoordinates[i][1] + glm::ivec3(LPV_RESOLUTION)) / glm::vec3(VIRTUAL_VOXEL_ATLAS_SIZE);
			lpv->cascades[i].virtualPropagatedGridBlueUVMin = glm::vec3(virtualGridCoordinates[i][2]) / glm::vec3(VIRTUAL_VOXEL_ATLAS_SIZE);
			lpv->cascades[i].virtualPropagatedGridBlueUVMax = glm::vec3(virtualGridCoordinates[i][2] + glm::ivec3(LPV_RESOLUTION)) / glm::vec3(VIRTUAL_VOXEL_ATLAS_SIZE);


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
			lpvComputeInjection->render(frameInfo.commandBuffer, &push);
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
			for (int p = 0; p < lpv->propagationIterations; p++) {
				push.pingPongIndex = p % 2;
				lpvComputePropagation->render(frameInfo.commandBuffer, &push);
			}
			vkCmdBindDescriptorSets(
				frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_COMPUTE,
				lpvComputeTemporalBlendPipelineLayout->getPipelineLayout(),
				0,
				1,
				&lpvGenerationDataDescriptorSet[frameInfo.frameIndex],
				0,
				nullptr
			);
			lpvComputeTemporalBlend->bind(frameInfo.commandBuffer);
			LPVMix pushM{};
			pushM.virtualPropagatedGridRedCoords = push.virtualPropagatedGridRedCoords;
			pushM.virtualPropagatedGridGreenCoords = push.virtualPropagatedGridGreenCoords;
			pushM.virtualPropagatedGridBlueCoords = push.virtualPropagatedGridBlueCoords;
			pushM.pingPongIndex = (lpv->propagationIterations) % 2;
			pushM.temporalBlend = 1.0f / 8.0f; // higher values are faster updates, but more susceptible to flickering, lower values update slower, but flicker less
			lpvComputeTemporalBlendPush.push(frameInfo.commandBuffer, lpvComputeTemporalBlendPipelineLayout->getPipelineLayout(), &pushM);
			vkCmdDispatch(frameInfo.commandBuffer, LPV_RESOLUTION / 16U, LPV_RESOLUTION / 16U, LPV_RESOLUTION);
		}


		const float VFAOBounds = 50.0f;
		SECamera vfaoMapCamera;
		vfaoMapCamera.setViewDirection(frameInfo.camera.getPosition() + glm::vec3{ 0.0, 200.0, 0.0 }, { 0.0, -1.0, 0.0 }, {0.0, 0.0, 1.0});
		vfaoMapCamera.setOrthographicProjection(
			-VFAOBounds,
			VFAOBounds,
			-VFAOBounds,
			VFAOBounds,
			-220.000f, 380.000f
		);

		SkyLightComponent* skl;

		// iterate over candidates
		for (auto& a : frameInfo.level->getRegistry().view<SkyLightComponent>()) {
			Actor actor = { a, frameInfo.level.get() };

			skl = &actor.getComponent<SkyLightComponent>();
		}
		skl->vfao.vp = vfaoMapCamera.getProjection() * vfaoMapCamera.getView();


		{
			// avoid jaggedness on static objects when moving the camera by snapping the position to the nearest original pixel
			float halfRes = static_cast<float>(VFAO_MAP_RESOLUTION) / 2.0f;

			// pick random point, doesn't matter
			glm::vec4 originPoint = glm::vec4(0.0, 0.0, 0.0, 1.0);
			glm::vec4 clip = skl->vfao.vp * originPoint;

			clip /= clip.w;
			glm::vec2 full = glm::floor(clip * halfRes);
			glm::vec4 newClip = vfaoMapCamera.getInverseProjection() * glm::vec4(full / halfRes, clip.z, 1.0);
			newClip /= newClip.w;
			glm::vec4 newWorld = vfaoMapCamera.getInverseView() * newClip;
			glm::vec4 difference = originPoint - newWorld;
			vfaoMapCamera.setViewDirection(frameInfo.camera.getPosition() + glm::vec3{ 0.0, 200.0, 0.0 } + glm::vec3(difference), { 0.0, -1.0, 0.0 }, { 0.0, 0.0, 1.0 });
			skl->vfao.vp = vfaoMapCamera.getProjection() * vfaoMapCamera.getView();
		}

		// regular shadow map pass
		vfaoMapRenderPass->beginRenderPass(frameInfo.commandBuffer, vfaoMapFramebuffer);
		vfaoMapPipeline->bind(frameInfo.commandBuffer);

		frameInfo.level->getRegistry().view<Components::TransformComponent, Components::MeshComponent>().each(
			[&](auto& tfc, auto& msc) {
				SEModel* model = frameInfo.resourceSystem->getModel(msc.mesh);
				glm::mat4 mpv = skl->vfao.vp * tfc.getTransformMatrix();
				shadowPush.push(frameInfo.commandBuffer, vfaoMapPipelineLayout->getPipelineLayout(), &mpv);

				for (const auto& [mapTo, matAsset] : msc.materials) {
					SESurfaceMaterial* material = frameInfo.resourceSystem->getSurfaceMaterial(matAsset);
					if (material->translucent) continue; // no need to bind materials as vfao doesnt care about alpha masks
					model->bind(frameInfo.commandBuffer, mapTo);
					model->draw(frameInfo.commandBuffer);
				}
			}
		);

		vfaoMapRenderPass->endRenderPass(frameInfo.commandBuffer);
		vfaoComputeFields->render(frameInfo.commandBuffer, &skl->vfao.vp);
		vfaoBlurFieldsH->render(frameInfo.commandBuffer, nullptr);
		vfaoBlurFieldsV->render(frameInfo.commandBuffer, nullptr);
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


		SEFramebufferAttachmentCreateInfo vfaoDepthAttachmentCreateInfo{};
		vfaoDepthAttachmentCreateInfo.dimensions = glm::ivec3(VFAO_MAP_RESOLUTION, VFAO_MAP_RESOLUTION, 1);
		vfaoDepthAttachmentCreateInfo.framebufferFormat = VK_FORMAT_D16_UNORM;
		vfaoDepthAttachmentCreateInfo.framebufferType = SEFramebufferAttachmentType::Depth;
		vfaoDepthAttachmentCreateInfo.imageAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		vfaoDepthAttachmentCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		vfaoDepthAttachmentCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		vfaoDepthAttachmentCreateInfo.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		vfaoDepthAttachmentCreateInfo.sampleCount = VK_SAMPLE_COUNT_1_BIT;
		vfaoDepthAttachmentCreateInfo.linearFiltering = false;
		vfaoMapDepthAttachment = new SEFramebufferAttachment(seDevice, vfaoDepthAttachmentCreateInfo);
		SEFramebufferAttachmentCreateInfo vfaoVarianceAttachmentCreateInfo{};
		vfaoVarianceAttachmentCreateInfo.dimensions = glm::ivec3(VFAO_MAP_RESOLUTION, VFAO_MAP_RESOLUTION, 1);
		vfaoVarianceAttachmentCreateInfo.framebufferFormat = VK_FORMAT_R16G16_UNORM;
		vfaoVarianceAttachmentCreateInfo.framebufferType = SEFramebufferAttachmentType::Color;
		vfaoVarianceAttachmentCreateInfo.imageAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		vfaoVarianceAttachmentCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		vfaoVarianceAttachmentCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		vfaoVarianceAttachmentCreateInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vfaoVarianceAttachmentCreateInfo.sampleCount = VK_SAMPLE_COUNT_1_BIT;
		vfaoVarianceAttachmentCreateInfo.linearFiltering = true;
		vfaoMapVarianceAttachment = new SEFramebufferAttachment(seDevice, vfaoVarianceAttachmentCreateInfo);
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

		SEAttachmentInfo vfaoDepthAttachmentInfo{};
		vfaoDepthAttachmentInfo.framebufferAttachment = vfaoMapDepthAttachment;
		vfaoDepthAttachmentInfo.clear.depth = { 1.0, 0 };
		vfaoDepthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		vfaoDepthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		SEAttachmentInfo vfaoVarianceAttachmentInfo{};
		vfaoVarianceAttachmentInfo.framebufferAttachment = vfaoMapVarianceAttachment;
		vfaoVarianceAttachmentInfo.clear.color = { 0.0, 0.0 };
		vfaoVarianceAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		vfaoVarianceAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		vfaoMapRenderPass = new SERenderPass(seDevice, { vfaoDepthAttachmentInfo, vfaoVarianceAttachmentInfo });

	}

	void ShadowMapSystem::createFramebuffers() {
		shadowMapFramebuffer = new SEFramebuffer(seDevice, shadowMapRenderPass, { shadowMapAttachment });
		rsmFramebuffer = new SEFramebuffer(seDevice, rsmRenderPass, { rsmDepthAttachment, rsmNormalAttachment, rsmFluxAttachment });
		vfaoMapFramebuffer = new SEFramebuffer(seDevice, vfaoMapRenderPass, { vfaoMapDepthAttachment, vfaoMapVarianceAttachment });
	}

	void ShadowMapSystem::createDescriptorSetLayouts() {
		shadowMapDescriptorSetLayout = SEDescriptorSetLayout::Builder(seDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();
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
			.addBinding(10, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

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


		lpvComputeClearPipelineLayout = new SEPipelineLayout(seDevice, { }, { lpvGenerationDataDescriptorSetLayout->getDescriptorSetLayout() });
		lpvComputeClear = new SEComputePipeline(
			seDevice,
			lpvComputeClearPipelineLayout->getPipelineLayout(),
			{ SEShader(SEShaderType::Compute, "res/shaders/spirv/lpv_clear.csh.spv")}
		);
		lpvComputeTemporalBlendPush = SEPushConstant(sizeof(LPVMix), VK_SHADER_STAGE_COMPUTE_BIT);
		lpvComputeTemporalBlendPipelineLayout = new SEPipelineLayout(seDevice, { lpvComputeTemporalBlendPush.getRange() }, { lpvGenerationDataDescriptorSetLayout->getDescriptorSetLayout() });
		lpvComputeTemporalBlend = new SEComputePipeline(
			seDevice,
			lpvComputeTemporalBlendPipelineLayout->getPipelineLayout(),
			{ SEShader(SEShaderType::Compute, "res/shaders/spirv/lpv_temporal_blend.csh.spv") }
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

		vfaoMapPipelineLayout = new SEPipelineLayout(seDevice, { shadowPush.getRange() }, { });

		SEGraphicsPipelineConfigInfo vfaoPipelineConfigInfo{};
		vfaoPipelineConfigInfo.enableVertexDescriptions();
		// remove uvs, normals and tangents, we dont need them
		vfaoPipelineConfigInfo.attributeDescriptions.pop_back();
		vfaoPipelineConfigInfo.attributeDescriptions.pop_back();
		vfaoPipelineConfigInfo.attributeDescriptions.pop_back();
		vfaoPipelineConfigInfo.bindingDescriptions.pop_back();
		vfaoPipelineConfigInfo.bindingDescriptions.pop_back();
		vfaoPipelineConfigInfo.bindingDescriptions.pop_back();

		vfaoPipelineConfigInfo.renderPass = vfaoMapRenderPass->getRenderPass();
		vfaoPipelineConfigInfo.pipelineLayout = vfaoMapPipelineLayout->getPipelineLayout();
		vfaoMapPipeline = new SEGraphicsPipeline(
			seDevice,
			vfaoPipelineConfigInfo,
			{ SEShader(SEShaderType::Vertex, "res/shaders/spirv/vfao.vsh.spv"), SEShader(SEShaderType::Fragment, "res/shaders/spirv/vfao.fsh.spv") }
		);

		vfaoComputeFields = new SEPostProcessingEffect(
			seDevice,
			{ VFAO_MAP_RESOLUTION, VFAO_MAP_RESOLUTION },
			SEShader(SEShaderType::Fragment, "res/shaders/spirv/vfao_fields.fsh.spv"),
			seDescriptorPool,
			{ vfaoMapVarianceAttachment->getDescriptor() },
			VK_FORMAT_R16G16B16A16_UNORM,
			VK_IMAGE_VIEW_TYPE_2D
		);


		vfaoBlurFieldsH = new SEPostProcessingEffect(
			seDevice,
			{ VFAO_MAP_RESOLUTION, VFAO_MAP_RESOLUTION },
			SEShader(SEShaderType::Fragment, "res/shaders/spirv/gaussian_h.fsh.spv"),
			seDescriptorPool,
			{ vfaoComputeFields->getAttachment()->getDescriptor() },
			VK_FORMAT_R16G16B16A16_UNORM,
			VK_IMAGE_VIEW_TYPE_2D
		);
		vfaoBlurFieldsV = new SEPostProcessingEffect(
			seDevice,
			{ VFAO_MAP_RESOLUTION, VFAO_MAP_RESOLUTION },
			SEShader(SEShaderType::Fragment, "res/shaders/spirv/gaussian_v.fsh.spv"),
			seDescriptorPool,
			{ vfaoBlurFieldsH->getAttachment()->getDescriptor() },
			VK_FORMAT_R16G16B16A16_UNORM,
			VK_IMAGE_VIEW_TYPE_2D
		);
	}
}