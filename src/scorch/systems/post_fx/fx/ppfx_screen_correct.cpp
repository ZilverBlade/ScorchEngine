#pragma once
#include "ppfx_screen_correct.h"
#include <scorch/vkapi/swap_chain.h>

namespace ScorchEngine::PostFX {
	struct ScreenCorrectionPush {
		float ditherIntensity;
		float invGamma;
	};

	ScreenCorrection::ScreenCorrection(
		SEDevice& device, 
		glm::vec2 size,
		SEDescriptorPool& descriptorPool,
		SEFramebufferAttachment* inputAttachment, 
		SESwapChain* swapChain
	) : PostFX::Effect(device) {
		screenCorrection = new SEPostProcessingEffect(
			device,
			size,
			SEShader(SEShaderType::Fragment, "res/shaders/spirv/screen_correct.fsh.spv"),
			descriptorPool,
			{ inputAttachment },
			swapChain->getRenderPass()
		);
	}
	ScreenCorrection::~ScreenCorrection() {
		delete screenCorrection;
	}
	void ScreenCorrection::render(FrameInfo& frameInfo) {
		ScreenCorrectionPush push{};
		push.invGamma = 1.0f / 2.2f;
		push.ditherIntensity = 0.5f / 256.f; // 8 bit
		//push.ditherIntensity = 0.5f / 1024.f; // 10 bit

		screenCorrection->render(frameInfo, &push);
	}
	void ScreenCorrection::resize(glm::vec2 size, const std::vector<SEFramebufferAttachment*>& newInputAttachments) {
		screenCorrection->resize(size, newInputAttachments);
	}
}