#pragma once

#include <vulkan/vulkan.h>
#include <scorch/ecs/level.h>
#include <scorch/rendering/camera.h>

namespace ScorchEngine {
	class ResourceSystem;
	struct FrameInfo {
		VkCommandBuffer commandBuffer;
		uint32_t frameIndex;
		float frameTime;
		SECamera camera;
		VkDescriptorSet globalUBO;
		VkDescriptorSet sceneSSBO;
		VkDescriptorSet skyLight;
		VkDescriptorSet shadowMap;
		std::shared_ptr<Level> level;
		ResourceSystem* resourceSystem;
	};
}