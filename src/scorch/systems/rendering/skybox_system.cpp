#pragma once
#include "skybox_system.h"

#include <scorch/ecs/level.h>
#include <scorch/ecs/actor.h>
#include <scorch/ecs/components.h>


namespace ScorchEngine {
	SkyboxSystem::SkyboxSystem(
		SEDevice& device, 
		VkRenderPass renderPass,
		VkDescriptorSetLayout uboLayout, 
		VkDescriptorSetLayout ssboLayout,
		VkDescriptorSetLayout skyboxDescriptorLayout,
		VkSampleCountFlagBits msaaSamples
	) : seDevice(device) {
		pipelineLayout = new SEPipelineLayout(seDevice, {}, { uboLayout, ssboLayout, skyboxDescriptorLayout });
		SEGraphicsPipelineConfigInfo pipelineConfigInfo{};
		pipelineConfigInfo.disableDepthWrite();
		pipelineConfigInfo.renderPass = renderPass;
		pipelineConfigInfo.pipelineLayout = pipelineLayout->getPipelineLayout();
		pipelineConfigInfo.setSampleCount(msaaSamples);

		pipeline = new SEGraphicsPipeline(seDevice, {
			SEShader(SEShaderType::Vertex, "res/shaders/spirv/skybox.vsh.spv"),
			SEShader(SEShaderType::Fragment, "res/shaders/spirv/skybox.fsh.spv")
		}, pipelineConfigInfo);
	}
	SkyboxSystem::~SkyboxSystem()
	{
	}
	void SkyboxSystem::render(FrameInfo& frameInfo, VkDescriptorSet skyboxDescriptor) {
		pipeline->bind(frameInfo.commandBuffer);
		VkDescriptorSet sets[3]{
			frameInfo.globalUBO,
			frameInfo.sceneSSBO,
			skyboxDescriptor
		};
		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout->getPipelineLayout(),
			0,
			3,
			sets,
			0,
			nullptr
		);
		vkCmdDraw(frameInfo.commandBuffer, 36, 1, 0, 0);
	}
}