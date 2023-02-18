#pragma once
#include <scorch/vkapi/device.h>
#include <glm/glm.hpp>
#include <scorch/renderer/frame_info.h>

#include <scorch/vkapi/render_pass.h>
#include <scorch/vkapi/frame_buffer_attachment.h>
#include <scorch/vkapi/frame_buffer.h>

#include <scorch/vkapi/graphics_pipeline.h>
#include <scorch/vkapi/pipeline_layout.h>
#include <scorch/vkapi/push_constant.h>

namespace ScorchEngine {
	class RenderSystem {
	public:
		RenderSystem(SEDevice& device, glm::ivec3 size, VkDescriptorSetLayout uboLayout, VkDescriptorSetLayout ssboLayout);
		virtual ~RenderSystem();

		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;
		RenderSystem(RenderSystem&&) = delete;
		RenderSystem& operator=(RenderSystem&&) = delete;

		virtual void renderEarlyDepth(FrameInfo& frameInfo) {}
		virtual void renderOpaque(FrameInfo& frameInfo) {}
		virtual void renderTranslucent(FrameInfo& frameInfo) {}
		virtual void renderSkybox(FrameInfo& frameInfo) {}
		 
		virtual void compositeData(FrameInfo& frameInfo) {}
		 
		virtual void beginOpaquePass(FrameInfo& frameInfo) {}
		virtual void endOpaquePass(FrameInfo& frameInfo) {}
		virtual void beginTranslucentPass(FrameInfo& frameInfo) {}
		virtual void endTranslucentPass(FrameInfo& frameInfo) {}
		virtual void beginCompositionPass(FrameInfo& frameInfo) {}
		virtual void endCompositionPass(FrameInfo& frameInfo){}

		virtual SEFrameBufferAttachment* getColorAttachment() { return nullptr; }
		virtual SEFrameBufferAttachment* getDepthAttachment() { return nullptr; }
		virtual void resize(glm::ivec3 newSize) {}

		virtual SERenderPass* getOpaqueRenderPass() { return nullptr; }
		virtual SERenderPass* getTransparencyRenderPass() { return nullptr; }
		virtual SERenderPass* getCompositionRenderPass() { return nullptr; }

	protected:
		virtual void init(glm::ivec3 size);
		virtual void destroy();

		virtual void createFrameBufferAttachments(glm::ivec3 size) {}
		virtual void createRenderPasses() {}
		virtual void createFrameBuffers() {}
		virtual void createGraphicsPipelines(VkDescriptorSetLayout uboLayout, VkDescriptorSetLayout ssboLayout) {}

		SEDevice& seDevice;
	};
}