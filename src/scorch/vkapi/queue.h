#pragma once

#include <vulkan/vulkan.h>

namespace ScorchEngine {
	enum class SEQueueType {
		Graphics,
		Compute
	};
	class SEQueue {
	public:
		VkQueue queue;
	private:
		bool occupied = false;
		SEQueueType type;
		friend class SEDevice;
	};
}