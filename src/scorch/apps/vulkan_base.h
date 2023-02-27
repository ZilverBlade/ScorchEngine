#pragma once

#include <scorch/apps/app.h>

#include <scorch/vkapi/swap_chain.h>
#include <scorch/vkapi/renderer.h>
#include <scorch/vkapi/graphics_pipeline.h>
#include <scorch/vkapi/pipeline_layout.h>
#include <scorch/vkapi/descriptors.h>
#include <scorch/vkapi/buffer.h>
#include <scorch/rendering/global_ubo.h>
#include <scorch/rendering/scene_ssbo.h>

namespace ScorchEngine::Apps {
	struct InFlightRenderData {
		std::unique_ptr<SEBuffer> uboBuffer{};
		VkDescriptorSet uboDescriptorSet{};
		std::unique_ptr<SEBuffer> ssboBuffer{};
		VkDescriptorSet ssboDescriptorSet{};

		std::unique_ptr<SceneSSBO> sceneSSBO{};
	};
	class VulkanBaseApp : public App {
	public:
		VulkanBaseApp(const char* name);
		virtual ~VulkanBaseApp();

		virtual void run();
	protected:
		SERenderer* seRenderer;
		SESwapChain* seSwapChain;
		std::unique_ptr<SEDescriptorPool> inFlightPool{};
		std::unique_ptr<SEDescriptorPool> staticPool{};
		std::vector<InFlightRenderData> renderData{};
		std::unique_ptr<SEDescriptorSetLayout> globalUBODescriptorLayout{};
		std::unique_ptr<SEDescriptorSetLayout> sceneSSBODescriptorLayout{};
	};
}