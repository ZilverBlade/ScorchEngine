#pragma once
#include <scorch/rendering/frame_info.h>
#include <scorch/rendering/scene_ssbo.h>
#include <scorch/vkapi/descriptors.h>
#include <scorch/vkapi/graphics_pipeline.h>
#include <scorch/vkapi/pipeline_layout.h>

namespace ScorchEngine {
	class SkyboxSystem {
	public:
		SkyboxSystem(
			SEDevice& device,
			VkRenderPass renderPass,
			VkDescriptorSetLayout uboLayout,
			VkDescriptorSetLayout ssboLayout,
			VkDescriptorSetLayout skyboxDescriptorLayout,
			VkSampleCountFlagBits msaaSamples
		);
		~SkyboxSystem();

		SkyboxSystem(const SkyboxSystem&) = delete;
		SkyboxSystem& operator=(const SkyboxSystem&) = delete;

		void render(FrameInfo& frameInfo, VkDescriptorSet skyboxDescriptor);
	private:
		SEDevice& seDevice;
		SEPipelineLayout* pipelineLayout{};
		SEGraphicsPipeline* pipeline{};
	};
}