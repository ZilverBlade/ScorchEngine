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
		createFrameBufferAttachments(size);
		createRenderPasses();
		createFrameBuffers();
	}

	void ForwardRenderSystem::destroy() {
		delete depthAttachment;
		delete opaqueColorAttachment;
		if (sampleCount != VK_SAMPLE_COUNT_1_BIT)
			delete opaqueColorResolveAttachment;
		delete opaqueFrameBuffer;
		delete opaqueRenderPass;
		delete earlyDepthFrameBuffer;
		delete earlyDepthRenderPass;
	}

	void ForwardRenderSystem::renderEarlyDepth(FrameInfo& frameInfo) {
		earlyDepthRenderPass->beginRenderPass(frameInfo.commandBuffer, earlyDepthFrameBuffer);
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
		
		renderMeshes(frameInfo, push, earlyDepthPipelineLayout->getPipelineLayout(), 1, false, true);

		earlyDepthRenderPass->endRenderPass(frameInfo.commandBuffer);
	}
	void ForwardRenderSystem::renderOpaque(FrameInfo& frameInfo) {
		opaquePipeline->bind(frameInfo.commandBuffer);
		VkDescriptorSet sets[2]{
			frameInfo.globalUBO,
			frameInfo.sceneSSBO
		};

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			opaquePipelineLayout->getPipelineLayout(),
			0,
			2,
			sets,
			0,
			nullptr
		);

		renderMeshes(frameInfo, push, opaquePipelineLayout->getPipelineLayout(), 3, false, true);
	}

	void ForwardRenderSystem::beginOpaquePass(FrameInfo& frameInfo) {
		opaqueRenderPass->beginRenderPass(frameInfo.commandBuffer, opaqueFrameBuffer);
	}
	void ForwardRenderSystem::endOpaquePass(FrameInfo& frameInfo) {
		opaqueRenderPass->endRenderPass(frameInfo.commandBuffer);
	}

	void ForwardRenderSystem::resize(glm::vec2 size) {
		earlyDepthFrameBuffer->resize(glm::ivec3(size, 1), earlyDepthRenderPass);
		opaqueFrameBuffer->resize(glm::ivec3(size, 1), opaqueRenderPass);
	}

	void ForwardRenderSystem::createFrameBufferAttachments(glm::vec2 size) {
		SEFrameBufferAttachmentCreateInfo depthAttachmentCreateInfo{};
		depthAttachmentCreateInfo.dimensions = glm::ivec3(size, 1);
		depthAttachmentCreateInfo.frameBufferFormat = VK_FORMAT_D32_SFLOAT;
		depthAttachmentCreateInfo.frameBufferType = SEFrameBufferAttachmentType::Depth;
		depthAttachmentCreateInfo.imageAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		depthAttachmentCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		depthAttachmentCreateInfo.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachmentCreateInfo.sampleCount = sampleCount;
		depthAttachmentCreateInfo.linearFiltering = false;
		depthAttachment = new SEFrameBufferAttachment(seDevice, depthAttachmentCreateInfo);

		SEFrameBufferAttachmentCreateInfo colorAttachmentCreateInfo{};
		colorAttachmentCreateInfo.dimensions = glm::ivec3(size, 1);
		colorAttachmentCreateInfo.frameBufferFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		colorAttachmentCreateInfo.frameBufferType = SEFrameBufferAttachmentType::Color;
		colorAttachmentCreateInfo.imageAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		colorAttachmentCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		colorAttachmentCreateInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		colorAttachmentCreateInfo.sampleCount = sampleCount;
		colorAttachmentCreateInfo.linearFiltering = false;
		opaqueColorAttachment = new SEFrameBufferAttachment(seDevice, colorAttachmentCreateInfo);

		if (sampleCount != VK_SAMPLE_COUNT_1_BIT) {
			SEFrameBufferAttachmentCreateInfo colorResolveAttachmentCreateInfo{};
			colorResolveAttachmentCreateInfo.dimensions = glm::ivec3(size, 1);
			colorResolveAttachmentCreateInfo.frameBufferFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
			colorResolveAttachmentCreateInfo.frameBufferType = SEFrameBufferAttachmentType::Resolve;
			colorResolveAttachmentCreateInfo.imageAspect = VK_IMAGE_ASPECT_COLOR_BIT;
			colorResolveAttachmentCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			colorResolveAttachmentCreateInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			colorResolveAttachmentCreateInfo.sampleCount = VK_SAMPLE_COUNT_1_BIT;
			colorResolveAttachmentCreateInfo.linearFiltering = false;
			opaqueColorResolveAttachment = new SEFrameBufferAttachment(seDevice, colorResolveAttachmentCreateInfo);
		}
	}

	void ForwardRenderSystem::createRenderPasses() {
		SEAttachmentInfo depthEarlyAttachmentInfo{};
		depthEarlyAttachmentInfo.frameBufferAttachment = depthAttachment;
		depthEarlyAttachmentInfo.clear.depth = { 1.0, 0 };
		depthEarlyAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthEarlyAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		earlyDepthRenderPass = new SERenderPass(seDevice, { depthEarlyAttachmentInfo });


		SEAttachmentInfo depthAttachmentInfo{};
		depthAttachmentInfo.frameBufferAttachment = depthAttachment;
		depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		SEAttachmentInfo opaqueColorAttachmentInfo{};
		opaqueColorAttachmentInfo.frameBufferAttachment = opaqueColorAttachment;
		opaqueColorAttachmentInfo.clear.color = { 0.0, 0.0, 0.0, 1.0 };
		opaqueColorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		opaqueColorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		SEAttachmentInfo opaqueColorResolveAttachmentInfo{};
		if (sampleCount != VK_SAMPLE_COUNT_1_BIT) {
			opaqueColorResolveAttachmentInfo.frameBufferAttachment = opaqueColorResolveAttachment;
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

	void ForwardRenderSystem::createFrameBuffers() {
		std::vector<SEFrameBufferAttachment*> attachments{};
		attachments.push_back(depthAttachment);
		attachments.push_back(opaqueColorAttachment);
		if (sampleCount != VK_SAMPLE_COUNT_1_BIT) {
			attachments.push_back(opaqueColorResolveAttachment);
		}
		opaqueFrameBuffer = new SEFrameBuffer(seDevice, opaqueRenderPass, attachments);
		earlyDepthFrameBuffer = new SEFrameBuffer(seDevice, earlyDepthRenderPass, { depthAttachment });
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
			{ SEShader(SEShaderType::Vertex, "res/shaders/spirv/depth.vsh.spv"), SEShader(SEShaderType::Fragment, "res/shaders/spirv/depth.fsh.spv") },
			earlyDepthPipelineConfigInfo
		);

		std::vector<VkDescriptorSetLayout> setLayouts = descriptorSetLayouts;
		setLayouts.push_back(MaterialSystem::getMaterialDescriptorSetLayout()->getDescriptorSetLayout());
		opaquePipelineLayout = new SEPipelineLayout(seDevice, { push.getRange() }, 
			setLayouts);

		SEGraphicsPipelineConfigInfo pipelineConfigInfo{};
		pipelineConfigInfo.enableVertexDescriptions();
		pipelineConfigInfo.setSampleCount(sampleCount);
		//pipelineConfigInfo.setCullMode(VK_CULL_MODE_BACK_BIT);
		pipelineConfigInfo.disableDepthWrite(); // early depth pass already takes care of depth writing
		pipelineConfigInfo.renderPass = opaqueRenderPass->getRenderPass();
		pipelineConfigInfo.pipelineLayout = opaquePipelineLayout->getPipelineLayout();
		opaquePipeline = new SEGraphicsPipeline(
			seDevice,
			{ SEShader(SEShaderType::Vertex, "res/shaders/spirv/model.vsh.spv"), SEShader(SEShaderType::Fragment, "res/shaders/spirv/forward_shading.fsh.spv") },
			pipelineConfigInfo
		);

		
	}
}