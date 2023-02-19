#pragma once

#include <scorch/apps/vulkan_base.h>
#include <scorch/ecs/level.h>
#include <scorch/ecs/actor.h>

namespace ScorchEngine::Apps {
	class LightingTest : virtual public VulkanBaseApp {
	public:
		LightingTest(const char* name);
		virtual ~LightingTest();

		virtual void run();
	protected:
		std::shared_ptr<Level> level;
	};
}