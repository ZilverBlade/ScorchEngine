#pragma once
#include "ppfx_screen_correct.h"
#include <scorch/vkapi/swap_chain.h>

namespace ScorchEngine::PostFX {
	struct ScreenCorrectionPush {
		float ditherIntensity;
		float invGamma;
	};

	ScreenCorrection::ScreenCorrection(
		SEDevice& device, 
		glm::vec2 size,
		SEFramebufferAttachment* inputAttachment, 
		SESwapChain* swapChain
	) : PostFX::Effect(device), inputAttachment(inputAttachment) {
		
		ppfxSceneDescriptorLayout = SEDescriptorSetLayout::Builder(seDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();
		createSceneDescriptors();
		createPipelineLayout();
		createPipeline(SEShader(SEShaderType::Fragment, "res/shaders/spirv/screen_correct.fsh.spv"), swapChain->getRenderPass());
	}
	ScreenCorrection::~ScreenCorrection() {

	}
	void ScreenCorrection::render(FrameInfo& frameInfo) {
		ScreenCorrectionPush push{};
		push.invGamma = 1.0f / 2.2f;
		push.ditherIntensity = 0.5f / 256.f; // 8 bit
		//push.ditherIntensity = 0.5f / 1024.f; // 10 bit

		ppfxPipeline->bind(frameInfo.commandBuffer);
		ppfxPush.push(frameInfo.commandBuffer, ppfxPipelineLayout->getPipelineLayout(), &push);
		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			ppfxPipelineLayout->getPipelineLayout(),
			0,
			1,
			&ppfxSceneDescriptorSet,
			0,
			nullptr
		);
		vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
	}
	void ScreenCorrection::resize(glm::vec2 size, const std::vector<SEFramebufferAttachment*>& newInputAttachments) {
		inputAttachment = newInputAttachments[0]; // only 1 scene input attachment
		createSceneDescriptors();
	}
	void ScreenCorrection::createPipelineLayout() {
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
			ppfxSceneDescriptorLayout->getDescriptorSetLayout()
		};
		ppfxPush = SEPushConstant(128, VK_SHADER_STAGE_FRAGMENT_BIT);
		std::vector<VkPushConstantRange> pushConstantRanges{
			ppfxPush.getRange()
		};
		ppfxPipelineLayout = std::make_unique<SEPipelineLayout>(seDevice, pushConstantRanges, descriptorSetLayouts);
	}
	void ScreenCorrection::createPipeline(const SEShader& fragmentShader, VkRenderPass renderPass) {
		SEGraphicsPipelineConfigInfo pipelineConfig{};
		pipelineConfig.setSampleCount(VK_SAMPLE_COUNT_1_BIT);
		//pipelineConfig.setCullMode(VK_CULL_MODE_BACK_BIT);
		pipelineConfig.disableDepthTest();

		pipelineConfig.pipelineLayout = ppfxPipelineLayout->getPipelineLayout();
		pipelineConfig.renderPass = renderPass;
		ppfxPipeline = std::make_unique<SEGraphicsPipeline>(
			seDevice,		pipelineConfig, std::vector<SEShader>{
			SEShader{ SEShaderType::Vertex, "res/shaders/spirv/full_screen.vsh.spv" },
				fragmentShader
		}
		);
	}
	void ScreenCorrection::createSceneDescriptors() {
		VkDescriptorImageInfo imageInfo = inputAttachment->getDescriptor();
		SEDescriptorWriter(*ppfxSceneDescriptorLayout, seDevice.getDescriptorPool())
			.writeImage(0, &imageInfo)
			.build(ppfxSceneDescriptorSet);
	}
}