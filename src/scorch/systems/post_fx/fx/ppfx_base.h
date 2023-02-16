#pragma once
#include <scorch/systems/post_fx/post_processing_fx.h>

namespace ScorchEngine {
	namespace PostFX {
		class Effect {
		public:
			Effect(
				SEDevice& device
			) : seDevice(device) {}
			virtual ~Effect() {}
			virtual void render(FrameInfo& frameInfo) {}
			virtual void resize(glm::vec2 size, const std::vector<SEFrameBufferAttachment*>& newInputAttachments) {}
			// derived classes must implement this //virtual SceneBufferInputData getNext();
		protected:
			SEDevice& seDevice;
		};
	}
}