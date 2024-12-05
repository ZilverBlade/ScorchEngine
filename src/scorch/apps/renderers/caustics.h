#pragma once

#include <scorch/apps/vulkan_base.h>
#include <scorch/ecs/level.h>
#include <scorch/ecs/actor.h>

namespace ScorchEngine::Apps {
	class CausticsApp : virtual public VulkanBaseApp {
	public:
		CausticsApp(const char* name);
		virtual ~CausticsApp();

		virtual void run();
	protected:
		std::shared_ptr<Level> level;
	};
}