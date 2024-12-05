#pragma once

#include <scorch/apps/vulkan_base.h>
#include <scorch/ecs/level.h>
#include <scorch/ecs/actor.h>

namespace ScorchEngine::Apps {
	class GrassApp : virtual public VulkanBaseApp {
	public:
		GrassApp(const char* name);
		virtual ~GrassApp();

		virtual void run();
	protected:
		std::shared_ptr<Level> level;
	};
}