#pragma once
#include <scorch/systems/post_fx/fx/ppfx_base.h>

namespace ScorchEngine {
	class SESwapChain;
	namespace PostFX {
		class ScreenCorrection : public Effect {
		public:
			ScreenCorrection(SEDevice& device, glm::vec2 size, SEFramebufferAttachment* inputAttachment, SESwapChain* swapChain);
			virtual ~ScreenCorrection();
			virtual void render(FrameInfo& frameInfo);
			virtual void resize(glm::vec2 size, const std::vector<SEFramebufferAttachment*>& newInputAttachments);
			virtual SEFramebufferAttachment* getAttachment() { return nullptr; }
		protected:
			void createPipelineLayout();
			void createPipeline(const SEShader& fragmentShader, VkRenderPass renderPass);
			void createSceneDescriptors();

			SEFramebufferAttachment* inputAttachment;

			std::unique_ptr<SEGraphicsPipeline> ppfxPipeline{};
			std::unique_ptr<SEPipelineLayout> ppfxPipelineLayout{};
			SEPushConstant ppfxPush{};

			std::unique_ptr<SEDescriptorSetLayout> ppfxSceneDescriptorLayout{};
			VkDescriptorImageInfo ppfxDescriptorRenderTarget{};
			VkDescriptorSet ppfxSceneDescriptorSet{};
		};
	}
}