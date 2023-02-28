#pragma once
#include <scorch/rendering/frame_info.h>
#include <scorch/vkapi/device.h>
#include <scorch/vkapi/framebuffer_attachment.h>
#include <scorch/vkapi/graphics_pipeline.h>
#include <scorch/vkapi/descriptors.h>
#include <scorch/vkapi/pipeline_layout.h>
#include <scorch/vkapi/push_constant.h>

namespace ScorchEngine {
	class SEFramebuffer;
	class SERenderPass;
	class SEPostProcessingEffect {
	public:
		SEPostProcessingEffect(
			SEDevice& device,
			glm::vec2 resolution,
			const SEShader& fragmentShader,
			SEDescriptorPool& descriptorPool,
			const std::vector<VkDescriptorImageInfo>& inputAttachments,
			VkFormat framebufferFormat,
			VkImageViewType viewType,
			uint32_t layers = 1,
			uint32_t mipLevels = 1
		);
		~SEPostProcessingEffect();

		SEPostProcessingEffect(const SEPostProcessingEffect&) = delete;
		SEPostProcessingEffect& operator= (const SEPostProcessingEffect&) = delete;

		void render(VkCommandBuffer commandBuffer, const void* pushData, uint32_t layer = 0, uint32_t mipLevel = 0);
		void resize(glm::vec2 newResolution, const std::vector<VkDescriptorImageInfo>& inputAttachments);

		SEFramebufferAttachment* getAttachment() {
			return ppfxRenderTarget;
		}
	private:
		void createPipelineLayout();
		void createPipeline(const SEShader& fragmentShader);
		void createSceneDescriptors();
		void createRenderPass(glm::vec2 resolution);

		SEDevice& seDevice;
		SEDescriptorPool& descriptorPool;

		uint32_t mipLevels = 1;
		uint32_t layerCount = 1;

		std::vector<std::vector<SEFramebuffer*>> ppfxSubFramebuffers{};
		SERenderPass* ppfxRenderPass{};
		SEFramebufferAttachment* ppfxRenderTarget{};
		VkFormat ppfxFramebufferFormat;
		VkImageViewType ppfxFramebufferViewType;

		std::vector<VkDescriptorImageInfo> inputAttachments;

		std::unique_ptr<SEGraphicsPipeline> ppfxPipeline{};
		std::unique_ptr<SEPipelineLayout> ppfxPipelineLayout{};
		SEPushConstant ppfxPush{};

		std::unique_ptr<SEDescriptorSetLayout> ppfxSceneDescriptorLayout{};
		VkDescriptorImageInfo ppfxDescriptorRenderTarget{};
		VkDescriptorSet ppfxSceneDescriptorSet{};
	};
}