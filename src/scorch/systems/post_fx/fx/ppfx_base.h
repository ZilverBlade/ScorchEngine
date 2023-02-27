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

			Effect(const Effect&) = delete;
			Effect& operator=(const Effect&) = delete;

			virtual void render(FrameInfo& frameInfo) {}
			virtual void resize(glm::vec2 size, const std::vector<SEFramebufferAttachment*>& newInputAttachments) {}
			virtual SEFramebufferAttachment* getAttachment() { return nullptr; }
		protected:
			SEDevice& seDevice;
		};
	}
}