#pragma once
#include <scorch/rendering/frame_info.h>
#include <scorch/rendering/scene_ssbo.h>
#include <scorch/vkapi/descriptors.h>
#include <scorch/vkapi/graphics_pipeline.h>
#include <scorch/vkapi/pipeline_layout.h>

namespace ScorchEngine {
	class SETextureCube;
	class SkyLightSystem {
	public:
		SkyLightSystem(SEDevice& device, SEDescriptorPool& descriptorPool);
		~SkyLightSystem();

		SkyLightSystem(const SkyLightSystem&) = delete;
		SkyLightSystem& operator=(const SkyLightSystem&) = delete;

		void update(FrameInfo& frameInfo, SceneSSBO& sceneBuffer);
		VkDescriptorSet getEnvironmentMapDescriptor(int frameIndex) { return skyboxDescriptors[frameIndex]; }
	private:
		SEDevice& seDevice;
		SEDescriptorPool& seDescriptorPool;
		SEDescriptorSetLayout skyboxDescriptorSetLayout;

		SEPipelineLayout* pipelineLayout{};
		SEGraphicsPipeline* pipeline{};

		std::vector<VkDescriptorSet> skyboxDescriptors{};
		SETextureCube* environment{};
	};
}