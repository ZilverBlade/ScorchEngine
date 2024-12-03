#pragma once
#include <scorch/vkapi/push_constant.h>
#include <scorch/vkapi/device.h>
#include <scorch/vkapi/descriptors.h>

namespace ScorchEngine {
	class SEGraphicsPipeline;
	class SEPipelineLayout;
	struct FrameInfo;

	class VoxelSDFRenderSystem {
	public:
		VoxelSDFRenderSystem(SEDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout uboLayout, VkSampleCountFlagBits msaaSamples);
		~VoxelSDFRenderSystem();

		VoxelSDFRenderSystem(const VoxelSDFRenderSystem&) = delete;
		VoxelSDFRenderSystem& operator=(const VoxelSDFRenderSystem&) = delete;
		VoxelSDFRenderSystem(VoxelSDFRenderSystem&&) = delete;
		VoxelSDFRenderSystem& operator=(VoxelSDFRenderSystem&&) = delete;

		void renderSDFs(const FrameInfo& frameInfo);
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