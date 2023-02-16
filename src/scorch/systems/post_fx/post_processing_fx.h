#pragma once
#include <scorch/renderer/frame_info.h>
#include <scorch/vkapi/device.h>
#include <scorch/vkapi/frame_buffer_attachment.h>
#include <scorch/vkapi/graphics_pipeline.h>
#include <scorch/vkapi/descriptors.h>
#include <scorch/vkapi/pipeline_layout.h>
#include <scorch/vkapi/push_constant.h>

namespace ScorchEngine {
	class SEFrameBuffer;
	class SERenderPass;
	class SEPostProcessingEffect {
	public:
		SEPostProcessingEffect(
			SEDevice& device,
			glm::vec2 resolution,
			const SEShader& fragmentShader,
			SEDescriptorPool& descriptorPool,
			const std::vector<SEFrameBufferAttachment*>& inputTargets = std::vector<SEFrameBufferAttachment*>(),
			VkFormat frameBufferFormat = VK_FORMAT_R16G16B16A16_SFLOAT
		);
		~SEPostProcessingEffect();

		SEPostProcessingEffect(const SEPostProcessingEffect&) = delete;
		SEPostProcessingEffect& operator= (const SEPostProcessingEffect&) = delete;

		void render(FrameInfo& frameInfo, const void* pushData);
		void resize(glm::vec2 newResolution, const std::vector<SEFrameBufferAttachment*>& inputTargets = std::vector<SEFrameBufferAttachment*>());

		SEFrameBufferAttachment* getAttachment() {
			return ppfxRenderTarget;
		}
	private:
		void createPipelineLayout();
		void createPipeline(const SEShader& fragmentShader);
		void createSceneDescriptors();
		void createRenderPass(glm::vec2 resolution);

		SEDevice& seDevice;
		SEDescriptorPool& descriptorPool;

		SEFrameBuffer* ppfxFrameBuffer{};
		SERenderPass* ppfxRenderPass{};
		SEFrameBufferAttachment* ppfxRenderTarget{};
		VkFormat ppfxFrameBufferFormat;

		std::vector<SEFrameBufferAttachment*> inputAttachments;

		std::unique_ptr<SEGraphicsPipeline> ppfxPipeline{};
		std::unique_ptr<SEPipelineLayout> ppfxPipelineLayout{};
		SEPushConstant ppfxPush{};

		std::unique_ptr<SEDescriptorSetLayout> ppfxSceneDescriptorLayout{};
		VkDescriptorImageInfo ppfxDescriptorRenderTarget{};
		VkDescriptorSet ppfxSceneDescriptorSet{};
	};
}