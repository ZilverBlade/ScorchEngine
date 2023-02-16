#include "post_processing_fx.h"
#include <scorch/vkapi/frame_buffer.h>
#include <scorch/vkapi/render_pass.h>

namespace ScorchEngine {
	SEPostProcessingEffect::SEPostProcessingEffect(
		SEDevice& device,
		glm::vec2 resolution,
		const SEShader& fragmentShader,
		SEDescriptorPool& descriptorPool,
		const std::vector<SEFrameBufferAttachment*>& inputAttachments,
		VkFormat frameBufferFormat
	) : seDevice(device), inputAttachments(inputAttachments), ppfxFrameBufferFormat(frameBufferFormat), descriptorPool(descriptorPool) {
		SEDescriptorSetLayout::Builder builder = SEDescriptorSetLayout::Builder(seDevice);

		for (int i = 0; i < inputAttachments.size(); i++) {
			builder.addBinding(i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		}

		ppfxSceneDescriptorLayout = builder.build();

		createSceneDescriptors();
		createRenderPass(resolution);
		createPipelineLayout();
		createPipeline(fragmentShader);
	}

	SEPostProcessingEffect::~SEPostProcessingEffect() {
		delete ppfxRenderTarget;
		delete ppfxRenderPass;
		delete ppfxFrameBuffer;
	}

	void SEPostProcessingEffect::render(FrameInfo& frameInfo, const void* pushData) {
		ppfxRenderPass->beginRenderPass(frameInfo.commandBuffer, ppfxFrameBuffer);
		ppfxPipeline->bind(frameInfo.commandBuffer);
		vkCmdBindDescriptorSets(frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			ppfxPipelineLayout->getPipelineLayout(),
			0,
			1,
			&ppfxSceneDescriptorSet,
			0,
			nullptr
		);
		ppfxPush.push(frameInfo.commandBuffer, ppfxPipelineLayout->getPipelineLayout(), pushData);
		vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
		ppfxRenderPass->endRenderPass(frameInfo.commandBuffer);
	}

	void SEPostProcessingEffect::resize(glm::vec2 newResolution, const std::vector<SEFrameBufferAttachment*>& newInputAttachments) {
		inputAttachments = newInputAttachments;
		createRenderPass(newResolution);
		createSceneDescriptors();
	}

	void SEPostProcessingEffect::createPipelineLayout() {
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
			ppfxSceneDescriptorLayout->getDescriptorSetLayout()
		};
		ppfxPush = SEPushConstant(128, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		std::vector<VkPushConstantRange> pushConstantRanges{
			ppfxPush.getRange()
		};
		ppfxPipelineLayout = std::make_unique<SEPipelineLayout>(seDevice, pushConstantRanges, descriptorSetLayouts);
	}

	void SEPostProcessingEffect::createPipeline(const SEShader& fragmentShader) {
		SEGraphicsPipelineConfigInfo pipelineConfig{};
		pipelineConfig.setCullMode(VK_CULL_MODE_BACK_BIT);
		pipelineConfig.disableDepthTest();

		pipelineConfig.pipelineLayout = ppfxPipelineLayout->getPipelineLayout();
		pipelineConfig.renderPass = ppfxRenderPass->getRenderPass();
		ppfxPipeline = std::make_unique<SEGraphicsPipeline>(
			seDevice, std::vector<SEShader>{
			SEShader{ SEShaderType::Vertex, "res/shaders/spirv/full_screen.vsh.spv" },
				fragmentShader
			},
			pipelineConfig
		);
	}

	void SEPostProcessingEffect::createSceneDescriptors() {
		auto writer = SEDescriptorWriter(*ppfxSceneDescriptorLayout, descriptorPool);
		for (int i = 0; i < inputAttachments.size(); i++) {
			writer.writeImage(i, &inputAttachments[i]->getDescriptor());
		}

		writer.build(ppfxSceneDescriptorSet);
	}
	void SEPostProcessingEffect::createRenderPass(glm::vec2 resolution) {
		if (ppfxRenderTarget) delete ppfxRenderTarget;
		if (ppfxRenderPass) delete ppfxRenderPass;
		if (ppfxFrameBuffer) delete ppfxFrameBuffer;

		glm::ivec3 windowSize = { resolution, 1 };
		ppfxRenderTarget = new SEFrameBufferAttachment(seDevice, {
				ppfxFrameBufferFormat,
				VK_IMAGE_ASPECT_COLOR_BIT,
				windowSize,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				SEFrameBufferAttachmentType::Color,
			}
		);

		ppfxRenderPass = new SERenderPass(seDevice, { AttachmentInfo{ppfxRenderTarget, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE }});
		ppfxFrameBuffer = new SEFrameBuffer(seDevice, ppfxRenderPass, { ppfxRenderTarget });
	}
}


