#pragma once
#include <scorch/vkapi/device.h>
#include <glm/glm.hpp>
#include <scorch/rendering/frame_info.h>

#include <scorch/vkapi/render_pass.h>
#include <scorch/vkapi/framebuffer_attachment.h>
#include <scorch/vkapi/framebuffer.h>

#include <scorch/vkapi/buffer.h>
#include <scorch/vkapi/empty_texture.h>
#include <scorch/vkapi/graphics_pipeline.h>
#include <scorch/vkapi/compute_pipeline.h>
#include <scorch/vkapi/pipeline_layout.h>
#include <scorch/vkapi/push_constant.h>

#include <scorch/systems/material_system.h>
#include <scorch/systems/post_fx/post_processing_fx.h>

namespace ScorchEngine {

	class WaterSystem {
	public:
		WaterSystem(SEDevice& device, 
			SEDescriptorPool& descriptorPool,
			VkRenderPass renderPass,
			std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
			VkSampleCountFlagBits msaaSamples);
		~WaterSystem();

		void render(FrameInfo& frameInfo);
	private:
		void init();
		void destroy();

		void createGraphicsPipelines();

		SEDevice& seDevice;
		SEDescriptorPool& seDescriptorPool;

		SEPushConstant waterPush{};
		SEPipelineLayout* waterPipelineLayout{};
		SEGraphicsPipeline* waterPipeline{};
	};
}