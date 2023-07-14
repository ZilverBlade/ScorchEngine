#include "forward_render_system.h"

#include <scorch/ecs/components.h>
#include <scorch/systems/resource_system.h>

namespace ScorchEngine {
	ForwardRenderSystem::ForwardRenderSystem(
		SEDevice& device, 
		glm::vec2 size,
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
		VkSampleCountFlagBits msaaSamples
	) : RenderSystem(device, size), sampleCount(msaaSamples)
	{
		init(size);
		createGraphicsPipelines(descriptorSetLayouts);
	}
	ForwardRenderSystem::~ForwardRenderSystem() {
		destroy();
		delete opaquePipeline;
		delete opaquePipelineLayout;
		delete earlyDepthPipeline;
		delete earlyDepthPipelineLayout;
	}

	void ForwardRenderSystem::init(glm::vec2 size) {
		createFramebufferAttachments(size);
		createRenderPasses();
		createFramebuffers();
	}

	void ForwardRenderSystem::destroy() {
		delete depthAttachment;
		delete opaqueColorAttachment;
		if (sampleCount != VK_SAMPLE_COUNT_1_BIT)
			delete opaqueColorResolveAttachment;
		delete opaqueFramebuffer;
		delete opaqueRenderPass;
		delete earlyDepthFramebuffer;
		delete earlyDepthRenderPass;
	}

	void ForwardRenderSystem::renderEarlyDepth(FrameInfo& frameInfo) {
		earlyDepthRenderPass->beginRenderPass(frameInfo.commandBuffer, earlyDepthFramebuffer);
		earlyDepthPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			earlyDepthPipelineLayout->getPipelineLayout(),
			0,
			1,
			&frameInfo.globalUBO,
			0,
			nullptr
		);
		
		renderMeshes(frameInfo, push, earlyDepthPipelineLayout->getPipelineLayout(), 1, false, false);
		renderMeshes(frameInfo, push, earlyDepthPipelineLayout->getPipelineLayout(), 1, false, true);

		earlyDepthRenderPass->endRenderPass(frameInfo.commandBuffer);
	}
	void ForwardRenderSystem::renderOpaque(FrameInfo& frameInfo) {
		opaquePipeline->bind(frameInfo.commandBuffer);
		VkDescriptorSet sets[4]{
			frameInfo.globalUBO,
			frameInfo.sceneSSBO,
			frameInfo.skyLight,
			frameInfo.shadowMap
		};

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			opaquePipelineLayout->getPipelineLayout(),
			0,
			4,
			sets,
			0,
			nullptr
		);

		renderMeshes(frameInfo, push, opaquePipelineLayout->getPipelineLayout(), 4, false, false);
		renderMeshes(frameInfo, push, opaquePipelineLayout->getPipelineLayout(), 4, false, true);
	}

	void ForwardRenderSystem::renderTranslucent(FrameInfo& frameInfo) {

		translucentPipeline->bind(frameInfo.commandBuffer);
		VkDescriptorSet sets[4]{
			frameInfo.globalUBO,
			frameInfo.sceneSSBO,
			frameInfo.skyLight,
			frameInfo.shadowMap
		};

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			opaquePipelineLayout->getPipelineLayout(),
			0,
			4,
			sets,
			0,
			nullptr
		);

		renderMeshes(frameInfo, push, opaquePipelineLayout->getPipelineLayout(), 4, true, false);
		//renderMeshes(frameInfo, push, opaquePipelineLayout->getPipelineLayout(), 4, true, true);
	}

	void ForwardRenderSystem::beginOpaquePass(FrameInfo& frameInfo) {
		opaqueRenderPass->beginRenderPass(frameInfo.commandBuffer, opaqueFramebuffer);
	}
	void ForwardRenderSystem::endOpaquePass(FrameInfo& frameInfo) {
		opaqueRenderPass->endRenderPass(frameInfo.commandBuffer);
	}

	void ForwardRenderSystem::resize(glm::vec2 size) {
		earlyDepthFramebuffer->resize(glm::ivec3(size, 1), earlyDepthRenderPass);
		opaqueFramebuffer->resize(glm::ivec3(size, 1), opaqueRenderPass);
	}

	void ForwardRenderSystem::createFramebufferAttachments(glm::vec2 size) {
		SEFramebufferAttachmentCreateInfo depthAttachmentCreateInfo{};
		depthAttachmentCreateInfo.dimensions = glm::ivec3(size, 1);
		depthAttachmentCreateInfo.framebufferFormat = VK_FORMAT_D32_SFLOAT;
		depthAttachmentCreateInfo.framebufferType = SEFramebufferAttachmentType::Depth;
		depthAttachmentCreateInfo.imageAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		depthAttachmentCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		depthAttachmentCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		depthAttachmentCreateInfo.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachmentCreateInfo.sampleCount = sampleCount;
		depthAttachmentCreateInfo.linearFiltering = false;
		depthAttachment = new SEFramebufferAttachment(seDevice, depthAttachmentCreateInfo);

		SEFramebufferAttachmentCreateInfo colorAttachmentCreateInfo{};
		colorAttachmentCreateInfo.dimensions = glm::ivec3(size, 1);
		colorAttachmentCreateInfo.framebufferFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		colorAttachmentCreateInfo.framebufferType = SEFramebufferAttachmentType::Color;
		colorAttachmentCreateInfo.imageAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		colorAttachmentCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorAttachmentCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		colorAttachmentCreateInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		colorAttachmentCreateInfo.sampleCount = sampleCount;
		colorAttachmentCreateInfo.linearFiltering = false;
		opaqueColorAttachment = new SEFramebufferAttachment(seDevice, colorAttachmentCreateInfo);

		if (sampleCount != VK_SAMPLE_COUNT_1_BIT) {
			SEFramebufferAttachmentCreateInfo colorResolveAttachmentCreateInfo{};
			colorResolveAttachmentCreateInfo.dimensions = glm::ivec3(size, 1);
			colorResolveAttachmentCreateInfo.framebufferFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
			colorResolveAttachmentCreateInfo.framebufferType = SEFramebufferAttachmentType::Resolve;
			colorResolveAttachmentCreateInfo.imageAspect = VK_IMAGE_ASPECT_COLOR_BIT;
			colorResolveAttachmentCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorResolveAttachmentCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			colorResolveAttachmentCreateInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			colorResolveAttachmentCreateInfo.sampleCount = VK_SAMPLE_COUNT_1_BIT;
			colorResolveAttachmentCreateInfo.linearFiltering = false;
			opaqueColorResolveAttachment = new SEFramebufferAttachment(seDevice, colorResolveAttachmentCreateInfo);
		}
	}

	void ForwardRenderSystem::createRenderPasses() {
		SEAttachmentInfo depthEarlyAttachmentInfo{};
		depthEarlyAttachmentInfo.framebufferAttachment = depthAttachment;
		depthEarlyAttachmentInfo.clear.depth = { 1.0, 0 };
		depthEarlyAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthEarlyAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		earlyDepthRenderPass = new SERenderPass(seDevice, { depthEarlyAttachmentInfo });


		SEAttachmentInfo depthAttachmentInfo{};
		depthAttachmentInfo.framebufferAttachment = depthAttachment;
		depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		SEAttachmentInfo opaqueColorAttachmentInfo{};
		opaqueColorAttachmentInfo.framebufferAttachment = opaqueColorAttachment;
		opaqueColorAttachmentInfo.clear.color = { 0.0, 0.0, 0.0, 1.0 };
		opaqueColorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		opaqueColorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		SEAttachmentInfo opaqueColorResolveAttachmentInfo{};
		if (sampleCount != VK_SAMPLE_COUNT_1_BIT) {
			opaqueColorResolveAttachmentInfo.framebufferAttachment = opaqueColorResolveAttachment;
			opaqueColorResolveAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			opaqueColorResolveAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		}

		std::vector<SEAttachmentInfo> attachmentInfos{};
		attachmentInfos.push_back(depthAttachmentInfo);
		attachmentInfos.push_back(opaqueColorAttachmentInfo);
		if (sampleCount != VK_SAMPLE_COUNT_1_BIT) {
			attachmentInfos.push_back(opaqueColorResolveAttachmentInfo);
		}
		opaqueRenderPass = new SERenderPass(seDevice, attachmentInfos);
	}

	void ForwardRenderSystem::createFramebuffers() {
		std::vector<SEFramebufferAttachment*> attachments{};
		attachments.push_back(depthAttachment);
		attachments.push_back(opaqueColorAttachment);
		if (sampleCount != VK_SAMPLE_COUNT_1_BIT) {
			attachments.push_back(opaqueColorResolveAttachment);
		}
		opaqueFramebuffer = new SEFramebuffer(seDevice, opaqueRenderPass, attachments);
		earlyDepthFramebuffer = new SEFramebuffer(seDevice, earlyDepthRenderPass, { depthAttachment });
	}

	void ForwardRenderSystem::createGraphicsPipelines(std::vector<VkDescriptorSetLayout> descriptorSetLayouts) {
		push = SEPushConstant(sizeof(ModelMatrixPush), VK_SHADER_STAGE_VERTEX_BIT);

		earlyDepthPipelineLayout = new SEPipelineLayout(seDevice, { push.getRange() }, 
			{ descriptorSetLayouts[0], MaterialSystem::getMaterialDescriptorSetLayout()->getDescriptorSetLayout() });
		// early depth only cares about first (global ubo) set layout

		SEGraphicsPipelineConfigInfo earlyDepthPipelineConfigInfo{};
		earlyDepthPipelineConfigInfo.enableVertexDescriptions();
		earlyDepthPipelineConfigInfo.setSampleCount(sampleCount);
		//earlyDepthPipelineConfigInfo.setCullMode(VK_CULL_MODE_BACK_BIT);
		earlyDepthPipelineConfigInfo.renderPass = earlyDepthRenderPass->getRenderPass();
		earlyDepthPipelineConfigInfo.pipelineLayout = earlyDepthPipelineLayout->getPipelineLayout();
		earlyDepthPipeline = new SEGraphicsPipeline(
			seDevice,
			earlyDepthPipelineConfigInfo,
			{ SEShader(SEShaderType::Vertex, "res/shaders/spirv/depth.vsh.spv"), SEShader(SEShaderType::Fragment, "res/shaders/spirv/depth.fsh.spv") }
		);

		std::vector<VkDescriptorSetLayout> setLayouts = descriptorSetLayouts;
		setLayouts.push_back(MaterialSystem::getMaterialDescriptorSetLayout()->getDescriptorSetLayout());
		opaquePipelineLayout = new SEPipelineLayout(seDevice, { push.getRange() }, setLayouts);

		SEGraphicsPipelineConfigInfo pipelineConfigInfo{};
		pipelineConfigInfo.enableVertexDescriptions();
		pipelineConfigInfo.setSampleCount(sampleCount);
		//pipelineConfigInfo.setCullMode(VK_CULL_MODE_BACK_BIT);
		pipelineConfigInfo.disableDepthWrite(); // early depth pass already takes care of depth writing
		pipelineConfigInfo.renderPass = opaqueRenderPass->getRenderPass();
		pipelineConfigInfo.pipelineLayout = opaquePipelineLayout->getPipelineLayout();
		opaquePipeline = new SEGraphicsPipeline(
			seDevice,
			pipelineConfigInfo,
			{ SEShader(SEShaderType::Vertex, "res/shaders/spirv/model.vsh.spv"), SEShader(SEShaderType::Fragment, "res/shaders/spirv/forward_shading.fsh.spv") }
		);

		SEGraphicsPipelineConfigInfo translucentPipelineConfigInfo{};
		translucentPipelineConfigInfo.enableVertexDescriptions();
		translucentPipelineConfigInfo.setSampleCount(sampleCount);
		pipelineConfigInfo.setCullMode(VK_CULL_MODE_BACK_BIT);
		translucentPipelineConfigInfo.disableDepthWrite(); // translucent objects must not write depth
		translucentPipelineConfigInfo.enableAlphaBlending();
		translucentPipelineConfigInfo.renderPass = opaqueRenderPass->getRenderPass();
		translucentPipelineConfigInfo.pipelineLayout = opaquePipelineLayout->getPipelineLayout();
		translucentPipeline = new SEGraphicsPipeline(
			seDevice,
			translucentPipelineConfigInfo,
			{ SEShader(SEShaderType::Vertex, "res/shaders/spirv/model.vsh.spv"), SEShader(SEShaderType::Fragment, "res/shaders/spirv/forward_shading.fsh.spv") }
		);
	}
}