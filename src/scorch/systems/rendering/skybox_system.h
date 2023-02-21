#pragma once
#include <scorch/renderer/frame_info.h>
#include <scorch/renderer/scene_ssbo.h>
#include <scorch/vkapi/descriptors.h>
#include <scorch/vkapi/graphics_pipeline.h>
#include <scorch/vkapi/pipeline_layout.h>

namespace ScorchEngine {
	class SETextureCube;
	class SkyboxSystem {
	public:
		SkyboxSystem(SEDevice& device, SEDescriptorPool& descriptorPool, SEDescriptorSetLayout& skyboxDescriptorLayout, VkRenderPass renderPass, VkDescriptorSetLayout uboLayout, VkDescriptorSetLayout ssboLayout, VkSampleCountFlagBits msaaSamples);
		~SkyboxSystem();

		SkyboxSystem(const SkyboxSystem&) = delete;
		SkyboxSystem& operator=(const SkyboxSystem&) = delete;

		void update(FrameInfo& frameInfo, SceneSSBO& sceneBuffer);
		void renderSkybox(FrameInfo& frameInfo);
		VkDescriptorSet getSkyboxDescriptor() { return skyboxDescriptor; }
	private:
		SEDevice& seDevice;
		SEDescriptorPool& seDescriptorPool;
		SEDescriptorSetLayout& skyboxDescriptorLayout;

		SEPipelineLayout* pipelineLayout{};
		SEGraphicsPipeline* pipeline{};

		VkDescriptorSet skyboxDescriptor = nullptr;
		SETextureCube* environment{};
	};
}