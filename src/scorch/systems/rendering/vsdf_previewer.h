#pragma once
#include <scorch/vkapi/push_constant.h>
#include <scorch/vkapi/device.h>
#include <scorch/vkapi/descriptors.h>

namespace ScorchEngine {
	class SEGraphicsPipeline;
	class SEPipelineLayout;
	struct FrameInfo;

	class VoxelSdfPreviewer {
	public:
		VoxelSdfPreviewer(SEDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout uboLayout, VkSampleCountFlagBits msaaSamples);
		~VoxelSdfPreviewer();

		VoxelSdfPreviewer(const VoxelSdfPreviewer&) = delete;
		VoxelSdfPreviewer& operator=(const VoxelSdfPreviewer&) = delete;
		VoxelSdfPreviewer(VoxelSdfPreviewer&&) = delete;
		VoxelSdfPreviewer& operator=(VoxelSdfPreviewer&&) = delete;

		void renderSdfs(const FrameInfo& frameInfo);
	protected:
		void createPipelineLayout(VkDescriptorSetLayout uboLayout);
		void createGraphicsPipeline(VkRenderPass renderPass, VkSampleCountFlagBits msaaSamples);

		SEDevice& seDevice;

		SEPushConstant push{};
		std::unique_ptr<SEDescriptorSetLayout> descriptorSetLayout;
		SEPipelineLayout* pipelineLayout{};
		SEGraphicsPipeline* pipeline{};
	};
}