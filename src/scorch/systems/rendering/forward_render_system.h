#pragma once
#include <scorch/systems/rendering/render_system.h>

namespace ScorchEngine {
	class ForwardRenderSystem final : public RenderSystem {
	public:
		ForwardRenderSystem(SEDevice& device, glm::vec2 size, std::vector<VkDescriptorSetLayout> descriptorSetLayouts, VkSampleCountFlagBits msaaSamples);
		virtual ~ForwardRenderSystem();

		virtual void renderEarlyDepth(FrameInfo& frameInfo) override;
		virtual void renderOpaque(FrameInfo& frameInfo) override;
		 
		virtual void beginOpaquePass(FrameInfo& frameInfo) override;
		virtual void endOpaquePass(FrameInfo& frameInfo) override;

		virtual void resize(glm::vec2 size) override;	
	protected:
		virtual void getColorAttachment(SEFrameBufferAttachment** out) override {
			*out = sampleCount == VK_SAMPLE_COUNT_1_BIT ? opaqueColorAttachment : opaqueColorResolveAttachment;
		}
		virtual void getDepthAttachment(SEFrameBufferAttachment** out) override { 
			*out = depthAttachment;
		}
		virtual void getOpaqueRenderPass(SERenderPass** out) override {
			*out = opaqueRenderPass;
		}


		virtual void init(glm::vec2 size) override;
		virtual void destroy() override;

		virtual void createFrameBufferAttachments(glm::vec2 size) override;
		virtual void createRenderPasses() override;
		virtual void createFrameBuffers() override;
		virtual void createGraphicsPipelines(std::vector<VkDescriptorSetLayout> descriptorSetLayouts) override;

		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;

		SEFrameBufferAttachment* depthAttachment{};
		SEFrameBufferAttachment* opaqueColorAttachment{};
		SEFrameBufferAttachment* opaqueColorResolveAttachment{};
		SEFrameBuffer* opaqueFrameBuffer{};
		SERenderPass* opaqueRenderPass{};

		SEPushConstant push{};
		SEPipelineLayout* opaquePipelineLayout{};
		SEGraphicsPipeline* opaquePipeline{};

		SEPipelineLayout* earlyDepthPipelineLayout{};
		SEGraphicsPipeline* earlyDepthPipeline{};
		SEFrameBuffer* earlyDepthFrameBuffer{};
		SERenderPass* earlyDepthRenderPass{};
	};
}