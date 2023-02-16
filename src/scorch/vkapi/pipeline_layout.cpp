#include "pipeline_layout.h"

namespace ScorchEngine {
	SEPipelineLayout::SEPipelineLayout(SEDevice& device, const std::vector<VkPushConstantRange>& pushConstantRanges, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts) : seDevice(device) {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = pushConstantRanges.size();
		pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();
		if (vkCreatePipelineLayout(seDevice.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}
	SEPipelineLayout::~SEPipelineLayout() {
		vkDestroyPipelineLayout(seDevice.getDevice(), pipelineLayout, nullptr);
	}
}