#include "vsdf_render_system.h"
#include <scorch/vkapi/pipeline_layout.h>
#include <scorch/vkapi/graphics_pipeline.h>
#include <scorch/vkapi/render_pass.h>
#include <scorch/vkapi/framebuffer.h>
#include <scorch/vkapi/framebuffer_attachment.h>
#include <scorch/rendering/frame_info.h>
#include <scorch/systems/resource_system.h>
#include <scorch/ecs/actor.h>
#include <scorch/graphics/sdf.h>
#include <glm/gtx/rotate_vector.hpp>

namespace ScorchEngine {
	struct Push {
		glm::mat4 invTransform;
		glm::vec3 halfExtent;
		int align;
		glm::vec3 scale;
		int align2;
		glm::vec3 toSun;
	};
	VoxelSdfRenderSystem::VoxelSdfRenderSystem(SEDevice& device, glm::vec2 size,
		VkDescriptorImageInfo depthTarget, VkDescriptorSetLayout uboLayout)
		: seDevice(device) {
		createRenderTargets(size);
		createRenderPasses();
		createFramebuffers();
		createPipelineLayout(uboLayout);
		createGraphicsPipeline();

		rebuildDepthDescriptor(depthTarget);
	}
	VoxelSdfRenderSystem::~VoxelSdfRenderSystem() {
		delete this->shadowMaskRenderPass;
		delete this->shadowMaskRenderTarget;
		delete this->shadowMaskFramebuffer;
		delete pipeline;
		delete pipelineLayout;
		seDevice.getDescriptorPool().freeDescriptors({ depthDescriptor });
	}
	void VoxelSdfRenderSystem::renderSdfShadows(const FrameInfo& frameInfo, Actor sun) {
		shadowMaskRenderPass->beginRenderPass(frameInfo.commandBuffer, shadowMaskFramebuffer);
		pipeline->bind(frameInfo.commandBuffer);
		for (entt::entity ent : frameInfo.level->getRegistry().view<TransformComponent, MeshComponent>()) {
			Actor actor = { ent, frameInfo.level.get() };
			SEModel* model = frameInfo.resourceSystem->getModel(actor.getComponent<Components::MeshComponent>().mesh);
			VkDescriptorSet descriptor = model->getSdf().getDescriptorSet();

			VkDescriptorSet descriptors[3]{
				frameInfo.globalUBO,
				descriptor,
				depthDescriptor
			};
			vkCmdBindDescriptorSets(
				frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout->getPipelineLayout(),
				0,
				3,
				descriptors,
				0,
				nullptr
			);
			auto& transform = actor.getTransform();
			Push pushData;
			glm::mat4 transformMat = transform.getTransformMatrix();
			transformMat[3] += glm::vec4(model->getSdf().getCenter(), 0.0f);
			pushData.invTransform = glm::inverse(transformMat);
			pushData.halfExtent = model->getSdf().getHalfExtent();
			pushData.scale = transform.scale;

			glm::mat4 tm = sun.getTransform().getTransformMatrix();
			tm = glm::rotate(tm, glm::half_pi<float>(), glm::vec3(1.f, 0.f, 0.f));
			glm::vec3 direction = glm::normalize(tm[2]); // convert rotation to direction
			pushData.toSun = -direction;

			push.push(frameInfo.commandBuffer, pipelineLayout->getPipelineLayout(), &pushData);
			vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
		}
		shadowMaskRenderPass->endRenderPass(frameInfo.commandBuffer);
	}
	VkDescriptorImageInfo VoxelSdfRenderSystem::getShadowMaskDescriptor() {
		return shadowMaskRenderTarget->getDescriptor();
	}
	void VoxelSdfRenderSystem::resize(glm::vec2 size, VkDescriptorImageInfo depthTarget) {
		shadowMaskFramebuffer->resize({ size * VSDF_SHADOW_QUALITY, 1 }, shadowMaskRenderPass);
		rebuildDepthDescriptor(depthTarget);
	}

	void VoxelSdfRenderSystem::createRenderTargets(glm::vec2 size) {
		SEFramebufferAttachmentCreateInfo shadowAttachmentCreateInfo{};
		shadowAttachmentCreateInfo.dimensions = glm::ivec3(size * VSDF_SHADOW_QUALITY, 1);
		shadowAttachmentCreateInfo.framebufferFormat = VK_FORMAT_R8_UNORM;
		shadowAttachmentCreateInfo.framebufferType = SEFramebufferAttachmentType::Color;
		shadowAttachmentCreateInfo.imageAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		shadowAttachmentCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		shadowAttachmentCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		shadowAttachmentCreateInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		shadowAttachmentCreateInfo.sampleCount = VK_SAMPLE_COUNT_1_BIT;
		shadowAttachmentCreateInfo.linearFiltering = false;
		shadowMaskRenderTarget = new SEFramebufferAttachment(seDevice, shadowAttachmentCreateInfo);
	}

	void VoxelSdfRenderSystem::createRenderPasses() {
		SEAttachmentInfo shadowAttachmentInfo{};
		shadowAttachmentInfo.framebufferAttachment = shadowMaskRenderTarget;
		shadowAttachmentInfo.clear.color = { 1.0 };
		shadowAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		shadowAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		shadowMaskRenderPass = new SERenderPass(seDevice, { shadowAttachmentInfo });
	}

	void VoxelSdfRenderSystem::createFramebuffers() {
		shadowMaskFramebuffer = new SEFramebuffer(seDevice, shadowMaskRenderPass, { shadowMaskRenderTarget });
	}

	void VoxelSdfRenderSystem::createPipelineLayout(VkDescriptorSetLayout uboLayout) {
		push = SEPushConstant(sizeof(Push), VK_SHADER_STAGE_FRAGMENT_BIT);
		sdfDescriptorSetLayout = SEDescriptorSetLayout::Builder(seDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();
		depthDescriptorSetLayout = SEDescriptorSetLayout::Builder(seDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();
		pipelineLayout = new SEPipelineLayout(
			seDevice,
			{ push.getRange() },
			{ uboLayout, depthDescriptorSetLayout->getDescriptorSetLayout(), sdfDescriptorSetLayout->getDescriptorSetLayout() }
		);
	}
	void VoxelSdfRenderSystem::createGraphicsPipeline() {
		SEGraphicsPipelineConfigInfo configInfo = SEGraphicsPipelineConfigInfo(1);
		configInfo.disableDepthTest();
		configInfo.disableDepthWrite();
		configInfo.renderPass = shadowMaskRenderPass->getRenderPass();
		configInfo.pipelineLayout = pipelineLayout->getPipelineLayout();
		configInfo.colorBlendAttachments[0].blendEnable = true;
		configInfo.colorBlendAttachments[0].colorBlendOp = VK_BLEND_OP_ADD;
		configInfo.colorBlendAttachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
		configInfo.colorBlendAttachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;

		pipeline = new SEGraphicsPipeline(seDevice, configInfo, {
			SEShader(SEShaderType::Vertex, "res/shaders/spirv/full_screen.vsh.spv"),
			SEShader(SEShaderType::Fragment, "res/shaders/spirv/vsdf_shadow.fsh.spv")
		});
	}
	void VoxelSdfRenderSystem::rebuildDepthDescriptor(VkDescriptorImageInfo depthTarget) {
		auto writer = SEDescriptorWriter(*depthDescriptorSetLayout, seDevice.getDescriptorPool())
			.writeImage(0, &depthTarget);
		if (depthDescriptor == VK_NULL_HANDLE) {
			writer.build(depthDescriptor);
		} else {
			writer.overwrite(depthDescriptor);
		}
	}
}