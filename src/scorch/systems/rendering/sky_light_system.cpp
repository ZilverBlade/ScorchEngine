#pragma once
#include "skybox_system.h"
#include <glm/gtx/rotate_vector.hpp>

#include <scorch/ecs/level.h>
#include <scorch/ecs/actor.h>
#include <scorch/ecs/components.h>

#include <scorch/systems/resource_system.h>

namespace ScorchEngine {
	SkyboxSystem::SkyboxSystem(
		SEDevice& device, 
		SEDescriptorPool& descriptorPool, 
		SEDescriptorSetLayout& skyboxDescriptorLayout,
		VkRenderPass renderPass,
		VkDescriptorSetLayout uboLayout, 
		VkDescriptorSetLayout ssboLayout, 
		VkSampleCountFlagBits msaaSamples
	) : seDevice(device), seDescriptorPool(descriptorPool), skyboxDescriptorLayout(skyboxDescriptorLayout) {
		pipelineLayout = new SEPipelineLayout(seDevice, {}, { uboLayout, ssboLayout, skyboxDescriptorLayout.getDescriptorSetLayout() });
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
	void SkyboxSystem::update(FrameInfo& frameInfo, SceneSSBO& sceneBuffer) {
		frameInfo.level->getRegistry().view<Components::SkyboxComponent>().each(
			[&](auto& skybox) {
				sceneBuffer.skybox.envTint = { skybox.envTint, skybox.envIntensity };
				sceneBuffer.skybox.tint = { skybox.tint, skybox.intensity };
				environment = frameInfo.resourceSystem->getTextureCube({ skybox.environmentMap, true, true });
			}
		);
		auto writer = SEDescriptorWriter::SEDescriptorWriter(skyboxDescriptorLayout, seDescriptorPool)
			.writeImage(0, &environment->getImageInfo());
		if (skyboxDescriptor) {
			writer.overwrite(skyboxDescriptor);
		} else {
			writer.build(skyboxDescriptor);
		}
	}
	void SkyboxSystem::renderSkybox(FrameInfo& frameInfo) {
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