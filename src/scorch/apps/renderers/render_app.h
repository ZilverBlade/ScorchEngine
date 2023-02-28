#pragma once

#include <scorch/apps/vulkan_base.h>
#include <scorch/ecs/level.h>
#include <scorch/ecs/actor.h>

namespace ScorchEngine::Apps {
	class RenderApp : virtual public VulkanBaseApp {
	public:
		RenderApp(const char* name);
		virtual ~RenderApp();

		virtual void run();
	protected:
		std::shared_ptr<Level> level;
	};
}