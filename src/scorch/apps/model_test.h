#pragma once

#include <scorch/apps/vulkan_base.h>
#include <scorch/ecs/level.h>
#include <scorch/ecs/actor.h>

namespace ScorchEngine::Apps {
	class ModelTest : virtual public VulkanBaseApp {
	public:
		ModelTest(const char* name);
		virtual ~ModelTest();

		virtual void run();
	protected:
		std::shared_ptr<Level> level;
	};
}