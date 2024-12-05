#pragma once

#include <scorch/apps/vulkan_base.h>
#include <scorch/ecs/level.h>
#include <scorch/ecs/actor.h>

namespace ScorchEngine::Apps {
	class SponzaApp : virtual public VulkanBaseApp {
	public:
		SponzaApp(const char* name);
		virtual ~SponzaApp();

		virtual void run();
	protected:
		std::shared_ptr<Level> level;
	};
}