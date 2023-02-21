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

#include <scorch/systems/material_system.h>

namespace ScorchEngine {
	struct ModelMatrixPush {
		glm::mat4 transform;
		glm::mat3 normal;
	};

	class RenderSystem {
	public:
		RenderSystem(SEDevice& device, glm::vec2 size);
		virtual ~RenderSystem();

		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;
		RenderSystem(RenderSystem&&) = delete;
		RenderSystem& operator=(RenderSystem&&) = delete;

		virtual void renderEarlyDepth(FrameInfo& frameInfo) {}
		virtual void renderOpaque(FrameInfo& frameInfo, VkDescriptorSet skyboxDescriptor) {}
		virtual void renderTranslucent(FrameInfo& frameInfo) {}
		virtual void renderSkybox(FrameInfo& frameInfo) {}
		 
		virtual void compositeData(FrameInfo& frameInfo) {}
		 
		virtual void beginOpaquePass(FrameInfo& frameInfo) {}
		virtual void endOpaquePass(FrameInfo& frameInfo) {}
		virtual void beginTranslucentPass(FrameInfo& frameInfo) {}
		virtual void endTranslucentPass(FrameInfo& frameInfo) {}
		virtual void beginCompositionPass(FrameInfo& frameInfo) {}
		virtual void endCompositionPass(FrameInfo& frameInfo){}

		SEFrameBufferAttachment* getColorAttachment() {
			SEFrameBufferAttachment* attachment{};
			getColorAttachment(&attachment);
			return attachment;
		}
		SEFrameBufferAttachment* getDepthAttachment() {
			SEFrameBufferAttachment* attachment{};
			getDepthAttachment(&attachment);
			return attachment;
		}
		virtual void resize(glm::vec2 size) {}

		SERenderPass* getOpaqueRenderPass() {
			SERenderPass* pass{};
			getOpaqueRenderPass(&pass);
			return pass;
		}
		SERenderPass* getTransparencyRenderPass() { return nullptr; }
		SERenderPass* getCompositionRenderPass() { return nullptr; }

	protected:
		void renderMeshes(FrameInfo& frameInfo, SEPushConstant push, VkPipelineLayout pipelineLayout, uint32_t descriptorOffset, bool translucent, bool doubleSided);

		virtual void getColorAttachment(SEFrameBufferAttachment** out) {}
		virtual void getDepthAttachment(SEFrameBufferAttachment** out) {}
		virtual void getOpaqueRenderPass(SERenderPass** out) {}

		virtual void init(glm::vec2 size);
		virtual void destroy();

		virtual void createFrameBufferAttachments(glm::vec2 size) {}
		virtual void createRenderPasses() {}
		virtual void createFrameBuffers() {}
		virtual void createGraphicsPipelines(std::vector<VkDescriptorSetLayout> descriptorSetLayouts) {}

		//std::unique_ptr<MaterialSystem> materialSystem;
		SEDevice& seDevice;
	};
}