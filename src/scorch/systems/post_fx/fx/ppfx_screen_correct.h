#pragma once
#include <scorch/systems/post_fx/fx/ppfx_base.h>

namespace ScorchEngine {
	class SESwapChain;
	namespace PostFX {
		class ScreenCorrection : public Effect {
		public:
			ScreenCorrection(SEDevice& device, glm::vec2 size, SEDescriptorPool& descriptorPool, SEFrameBufferAttachment* inputAttachment, SESwapChain* swapChain);
			virtual ~ScreenCorrection();
			virtual void render(FrameInfo& frameInfo);
			virtual void resize(glm::vec2 size, const std::vector<SEFrameBufferAttachment*>& newInputAttachments);
			virtual SEFrameBufferAttachment* getAttachment() { return screenCorrection->getAttachment(); }
		protected:
			SEPostProcessingEffect* screenCorrection{};
		};
	}
}