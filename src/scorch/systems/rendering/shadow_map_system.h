#pragma once
#include <scorch/vkapi/device.h>
#include <glm/glm.hpp>
#include <scorch/rendering/frame_info.h>

#include <scorch/vkapi/render_pass.h>
#include <scorch/vkapi/framebuffer_attachment.h>
#include <scorch/vkapi/framebuffer.h>

#include <scorch/vkapi/buffer.h>
#include <scorch/vkapi/voxel_texture.h>
#include <scorch/vkapi/graphics_pipeline.h>
#include <scorch/vkapi/compute_pipeline.h>
#include <scorch/vkapi/pipeline_layout.h>
#include <scorch/vkapi/push_constant.h>

#include <scorch/systems/material_system.h>
#include <scorch/systems/post_fx/post_processing_fx.h>

namespace ScorchEngine {
	const uint32_t SHADOW_MAP_RESOLUTION = 1024U;
	const uint32_t SHADOW_MAP_RSM_RESOLUTION = 256U;
	const uint32_t VIRTUAL_VOXEL_ATLAS_SIZE = 128U; // more than enough for several voxels
	const uint32_t LPV_RESOLUTION = 32U; // dont make different from multiple of 16 due to the dispatch config
	const uint32_t LPV_PROPGATION_FASES = 1U; // propagate light X times, higher is better propagation, keep at 1 due to the current setup

	class ShadowMapSystem {
	public:
		ShadowMapSystem(SEDevice& device, SEDescriptorPool& descriptorPool);
		~ShadowMapSystem();

		void render(FrameInfo& frameInfo);
		VkDescriptorSetLayout getDescriptorSetLayout() {
			return shadowMapDescriptorSetLayout->getDescriptorSetLayout();
		}
		VkDescriptorSet getDescriptorSet() {
			return shadowMapDescriptorSet;
		}
	private:	
		void init();
		void destroy();

		void createFramebufferAttachments();
		void createRenderPasses();
		void createFramebuffers();
		void createGraphicsPipelines();

		void createLPV();
		void writeDescriptor();

		SEDevice& seDevice;

		VkDescriptorSet shadowMapDescriptorSet{};
		std::unique_ptr<SEDescriptorSetLayout> shadowMapDescriptorSetLayout{};
		SEDescriptorPool& seDescriptorPool;

		SEFramebufferAttachment* shadowMapAttachment{};
		SEFramebuffer* shadowMapFramebuffer{};
		SERenderPass* shadowMapRenderPass{};

		SEPushConstant shadowPush{};
		SEPipelineLayout* shadowMapPipelineLayout{};
		SEGraphicsPipeline* shadowMapPipeline{};

		std::unique_ptr<SEBuffer> lpvInjectionData[MAX_FRAMES_IN_FLIGHT];
		SEVoxelTexture* lpvInoutRedSH{};
		SEVoxelTexture* lpvInoutGreenSH{};
		SEVoxelTexture* lpvInoutBlueSH{};
		// ping pong textures for the propagation fase
		SEVoxelTexture* lpvInout2RedSH{};
		SEVoxelTexture* lpvInout2GreenSH{};
		SEVoxelTexture* lpvInout2BlueSH{};
		SEVoxelTexture* lpvPropagatedAtlasSH{};

		std::unique_ptr<SEDescriptorSetLayout> lpvGenerationDataDescriptorSetLayout{};
		VkDescriptorSet lpvGenerationDataDescriptorSet[MAX_FRAMES_IN_FLIGHT];

		SEFramebufferAttachment* rsmDepthAttachment{};
		SEFramebufferAttachment* rsmNormalAttachment{};
		SEFramebufferAttachment* rsmFluxAttachment{};
		SEFramebuffer* rsmFramebuffer{};
		SERenderPass* rsmRenderPass{};
		SEPushConstant rsmPush{};
		SEPipelineLayout* rsmPipelineLayout{};
		SEGraphicsPipeline* rsmPipeline{};


		SEPipelineLayout* lpvComputeClearPipelineLayout{};
		SEComputePipeline* lpvComputeClear{};
		SEPushConstant lpvComputeTemporalBlendPush{};
		SEPipelineLayout* lpvComputeTemporalBlendPipelineLayout{};
		SEComputePipeline* lpvComputeTemporalBlend{};
		SEPostProcessingEffect* lpvComputeInjection{};
		SEPostProcessingEffect* lpvComputePropagation{};
	};
}