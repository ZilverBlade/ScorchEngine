#pragma once

#include <vulkan/vulkan.h>

namespace ScorchEngine {
	enum class SEQueueType {
		Present,
		Graphics,
		Compute
	};
	class SEQueue {
	public:
		VkQueue queue;
		SEQueueType type;
		bool inUse = false;
	};
}