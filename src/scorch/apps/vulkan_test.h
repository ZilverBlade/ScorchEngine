#pragma once

#include <scorch/apps/app.h>
#include <scorch/vkapi/swap_chain.h>
#include <scorch/vkapi/renderer.h>

namespace ScorchEngine {
	class VulkanTest : public App {
	public:
		VulkanTest(const char* name);
		virtual ~VulkanTest();

		virtual void run();
	protected:
		SERenderer seRenderer;
		SESwapChain seSwapChain;
	};
}