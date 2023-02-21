#pragma once
#include <scorch/renderer/frame_info.h>
#include <scorch/renderer/scene_ssbo.h>
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