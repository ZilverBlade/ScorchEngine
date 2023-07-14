#pragma once
#include <scorch/systems/rendering/render_system.h>

namespace ScorchEngine {
	class ForwardRenderSystem final : public RenderSystem {
	public:
		ForwardRenderSystem(SEDevice& device, glm::vec2 size, std::vector<VkDescriptorSetLayout> descriptorSetLayouts, VkSampleCountFlagBits msaaSamples);
		virtual ~ForwardRenderSystem();

		virtual void renderEarlyDepth(FrameInfo& frameInfo) override;
		virtual void renderOpaque(FrameInfo& frameInfo) override;
		virtual void renderTranslucent(FrameInfo& frameInfo) override;
		 
		virtual void beginOpaquePass(FrameInfo& frameInfo) override;
		virtual void endOpaquePass(FrameInfo& frameInfo) override;

		virtual void resize(glm::vec2 size) override;	
	protected:
		virtual void getColorAttachment(SEFramebufferAttachment** out) override {
			*out = (sampleCount == VK_SAMPLE_COUNT_1_BIT) ? opaqueColorAttachment : opaqueColorResolveAttachment;
		}
		virtual void getDepthAttachment(SEFramebufferAttachment** out) override { 
			*out = depthAttachment;
		}
		virtual void getOpaqueRenderPass(SERenderPass** out) override {
			*out = opaqueRenderPass;
		}

		virtual void init(glm::vec2 size) override;
		virtual void destroy() override;

		virtual void createFramebufferAttachments(glm::vec2 size) override;
		virtual void createRenderPasses() override;
		virtual void createFramebuffers() override;
		virtual void createGraphicsPipelines(std::vector<VkDescriptorSetLayout> descriptorSetLayouts) override;

		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;

		SEFramebufferAttachment* depthAttachment{};
		SEFramebufferAttachment* opaqueColorAttachment{};
		SEFramebufferAttachment* opaqueColorResolveAttachment{};
		SEFramebuffer* opaqueFramebuffer{};
		SERenderPass* opaqueRenderPass{};

		SEPushConstant push{};
		SEPipelineLayout* opaquePipelineLayout{};
		SEGraphicsPipeline* opaquePipeline{};
		SEGraphicsPipeline* translucentPipeline{};
		

		SEPipelineLayout* earlyDepthPipelineLayout{};
		SEGraphicsPipeline* earlyDepthPipeline{};
		SEFramebuffer* earlyDepthFramebuffer{};
		SERenderPass* earlyDepthRenderPass{};
	};
}