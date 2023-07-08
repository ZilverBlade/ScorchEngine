#include "compute_pipeline.h"
#include <cassert>

namespace ScorchEngine {
	SEComputePipeline::SEComputePipeline(SEDevice& device, VkPipelineLayout pipelineLayout, const SEShader& shader) : seDevice(device) {
		VkPipelineShaderStageCreateInfo shaderStage{};
		VkShaderModule module = shader.createShaderModule(seDevice);
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		shaderStage.module = module;
		shaderStage.pName = "main";
		shaderStage.flags = 0;
		shaderStage.pNext = nullptr;
		shaderStage.pSpecializationInfo = nullptr;

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.stage = shaderStage;
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.flags = 0;

		if (vkCreateComputePipelines(seDevice.getDevice(), nullptr, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute pipeline!");
		}
	}
	SEComputePipeline::~SEComputePipeline() {
		vkDestroyPipeline(seDevice.getDevice(), pipeline, nullptr);
	}
	void SEComputePipeline::bind(VkCommandBuffer commandBuffer) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	}
}