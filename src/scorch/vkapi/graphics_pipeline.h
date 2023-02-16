#pragma once
#include <scorch/vkapi/device.h>
#include <scorch/vkapi/shader.h>
namespace ScorchEngine {

	struct SEGraphicsPipelineConfigInfo {
		SEGraphicsPipelineConfigInfo(uint32_t colorAttachmentCount = 1);
		SEGraphicsPipelineConfigInfo(const SEGraphicsPipelineConfigInfo&) = delete;
		SEGraphicsPipelineConfigInfo& operator=(const SEGraphicsPipelineConfigInfo&) = delete;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
		VkPipelineViewportStateCreateInfo viewportInfo{};
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
		VkPipelineMultisampleStateCreateInfo multisampleInfo{};
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments{};
		VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
		std::vector<VkDynamicState> dynamicStateEnables{};
		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;

		//void enableVertexDescriptions(VertexDescriptionsFlags descriptions);

		void enableAlphaBlending(uint32_t attachment = 0, VkBlendOp blendOp = VK_BLEND_OP_ADD);
		void wireframe(float thickness = 1.0f);
		void setCullMode(VkCullModeFlags cullMode);
		void setSampleCount(VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
		void reverseDepth();
		void disableDepthTest();
		void disableDepthWrite();
	};

	class SEGraphicsPipeline {
	public:
		SEGraphicsPipeline(
			SEDevice& device,
			const std::vector<SEShader>& shaders,
			const SEGraphicsPipelineConfigInfo& configInfo
		);
		~SEGraphicsPipeline();
		
		SEGraphicsPipeline(const SEGraphicsPipeline&) = delete;
		SEGraphicsPipeline& operator=(const SEGraphicsPipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer);
	private:
		VkPipeline pipeline;
		SEDevice& seDevice;
	};

}