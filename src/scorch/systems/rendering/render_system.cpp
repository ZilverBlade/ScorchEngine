#include "render_system.h"

namespace ScorchEngine {
	RenderSystem::RenderSystem(
		SEDevice& device, 
		glm::vec2 size,
		VkDescriptorSetLayout uboLayout, 
		VkDescriptorSetLayout ssboLayout
	) : seDevice(device)
	{
	}
	RenderSystem::~RenderSystem() {
	}
	void RenderSystem::init(glm::vec2 size) {
	}
	void RenderSystem::destroy()
	{
	}
}