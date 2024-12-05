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
	const uint32_t SHADOW_MAP_RESOLUTION = 2048U;
	const uint32_t SHADOW_MAP_RSM_RESOLUTION = 256U;
	const uint32_t VIRTUAL_VOXEL_ATLAS_SIZE = 128U; // more than enough for several voxels
	const uint32_t LPV_RESOLUTION = 32U; // dont make different from multiple of 16 due to the dispatch config
	const uint32_t LPV_PROPGATION_FASES = 1U; // propagate light X times, higher is better propagation, keep at 1 due to the current setup
	const uint32_t VFAO_MAP_RESOLUTION = 1024U; // low resolution for large distance is fine as we only need the fields 

	class ShadowMapSystem {
	public:
		ShadowMapSystem(SEDevice& device);
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
		void createDescriptorSetLayouts();
		void createGraphicsPipelines();

		void createLPV();
		void writeDescriptor();

		SEDevice& seDevice;

		VkDescriptorSet shadowMapDescriptorSet{};
		std::unique_ptr<SEDescriptorSetLayout> shadowMapDescriptorSetLayout{};

		SEFramebufferAttachment* shadowMapAttachment{};
		SEFramebuffer* shadowMapFramebuffer{};
		SERenderPass* shadowMapRenderPass{};

		SEPushConstant shadowPush{};
		SEPipelineLayout* shadowMapPipelineLayout{};
		SEGraphicsPipeline* shadowMapPipeline{};

		VkDescriptorSet causticInjectionDescriptorSet{};
		std::unique_ptr<SEDescriptorSetLayout> causticInjectionDescriptorSetLayout{};
		SEFramebufferAttachment* causticMapRefractionAttachment{};
		SEFramebufferAttachment* causticMapFresnelAttachment{};
		SEFramebuffer* causticMapFramebuffer{};
		SERenderPass* causticMapRenderPass{};

		SEPushConstant causticPush{};
		SEPipelineLayout* causticMapPipelineLayout{};
		SEPipelineLayout* causticMapInjectionPipelineLayout{};
		SEGraphicsPipeline* causticMapPipeline{};
		SEPostProcessingEffect* causticComputeInjection{};
		SEPostProcessingEffect* causticScale{};
		SEPostProcessingEffect* causticBlurH{};
		SEPostProcessingEffect* causticBlurV{};
		SEEmptyTexture* causticValueMap;
		SEComputePipeline* causticClear{};

		SEFramebufferAttachment* vfaoMapDepthAttachment{};
		SEFramebufferAttachment* vfaoMapVarianceAttachment{};
		SEFramebuffer* vfaoMapFramebuffer{};
		SERenderPass* vfaoMapRenderPass{};

		SEPipelineLayout* vfaoMapPipelineLayout{};
		SEGraphicsPipeline* vfaoMapPipeline{};
		SEPostProcessingEffect* vfaoComputeFields{};
		SEPostProcessingEffect* vfaoBlurFieldsH{};
		SEPostProcessingEffect* vfaoBlurFieldsV{};

		std::unique_ptr<SEBuffer> lpvInjectionData[MAX_FRAMES_IN_FLIGHT];
		SEEmptyTexture* lpvInoutRedSH{};
		SEEmptyTexture* lpvInoutGreenSH{};
		SEEmptyTexture* lpvInoutBlueSH{};
		// ping pong textures for the propagation fase
		SEEmptyTexture* lpvInout2RedSH{};
		SEEmptyTexture* lpvInout2GreenSH{};
		SEEmptyTexture* lpvInout2BlueSH{};
		SEEmptyTexture* lpvPropagatedAtlasSH{};

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