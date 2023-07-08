#pragma once
#include <scorch/vkapi/device.h>
#include <scorch/vkapi/shader.h>
namespace ScorchEngine {
	class SEComputePipeline {
	public:
		SEComputePipeline(SEDevice& device, VkPipelineLayout pipelineLayout, const SEShader& shader);
		~SEComputePipeline();

		SEComputePipeline(const SEComputePipeline&) = delete;
		SEComputePipeline& operator=(const SEComputePipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer);
	private:
		SEDevice& seDevice;
		VkPipeline pipeline;
	};
}