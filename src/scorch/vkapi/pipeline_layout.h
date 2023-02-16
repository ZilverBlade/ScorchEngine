#pragma once
#include <scorch/vkapi/device.h>

namespace ScorchEngine {
	class SEPipelineLayout {
	public:
		SEPipelineLayout(SEDevice& device, const std::vector<VkPushConstantRange>& pushConstantRanges = std::vector<VkPushConstantRange>(), const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts = std::vector<VkDescriptorSetLayout>());
		~SEPipelineLayout();

		SEPipelineLayout(const SEPipelineLayout&) = delete;
		SEPipelineLayout& operator=(const SEPipelineLayout&) = delete;

		inline VkPipelineLayout getPipelineLayout() { return pipelineLayout; }
	private:
		VkPipelineLayout pipelineLayout{};
		SEDevice& seDevice;
	};
}