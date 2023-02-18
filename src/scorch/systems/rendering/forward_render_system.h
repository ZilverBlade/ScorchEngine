#pragma once
#include <scorch/systems/rendering/render_system.h>

namespace ScorchEngine {
	class ForwardRenderSystem final : public RenderSystem {
	public:
		ForwardRenderSystem(SEDevice& device, glm::ivec3 size, VkDescriptorSetLayout uboLayout, VkDescriptorSetLayout ssboLayout, VkSampleCountFlagBits msaaSamples,
			VkRenderPass real);
		virtual ~ForwardRenderSystem();

		virtual void renderEarlyDepth(FrameInfo& frameInfo) override;
		virtual void renderOpaque(FrameInfo& frameInfo) override;
		 
		virtual void beginOpaquePass(FrameInfo& frameInfo) override;
		virtual void endOpaquePass(FrameInfo& frameInfo) override;

		virtual SEFrameBufferAttachment* getColorAttachment() override { 
			return sampleCount == VK_SAMPLE_COUNT_1_BIT ? opaqueColorAttachment : opaqueColorResolveAttachment;
		}
		virtual SEFrameBufferAttachment* getDepthAttachment() override { return depthAttachment; }
		virtual void resize(glm::ivec3 newSize) override;

		virtual SERenderPass* getOpaqueRenderPass() override { return opaqueRenderPass; }
		
	protected:
		void iterateOpaqueObjects(FrameInfo& frameInfo);
		virtual void init(glm::ivec3 size) override;
		virtual void destroy() override;

		virtual void createFrameBufferAttachments(glm::ivec3 size) override;
		virtual void createRenderPasses() override;
		virtual void createFrameBuffers() override;
		virtual void createGraphicsPipelines(VkDescriptorSetLayout uboLayout, VkDescriptorSetLayout ssboLayout) override;

		VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;

		SEFrameBufferAttachment* depthAttachment{};
		SEFrameBufferAttachment* opaqueColorAttachment{};
		SEFrameBufferAttachment* opaqueColorResolveAttachment{};
		SEFrameBuffer* opaqueFrameBuffer{};
		SERenderPass* opaqueRenderPass{};

		SEPushConstant push{};
		SEPipelineLayout* opaquePipelineLayout{};
		SEGraphicsPipeline* opaquePipeline{};

		VkRenderPass real;

		SEPipelineLayout* earlyDepthPipelineLayout{};
		SEGraphicsPipeline* earlyDepthPipeline{};
		SEFrameBuffer* earlyDepthFrameBuffer{};
		SERenderPass* earlyDepthRenderPass{};
	};
}