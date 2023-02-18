#include "render_system.h"

namespace ScorchEngine {
	RenderSystem::RenderSystem(
		SEDevice& device, 
		glm::ivec3 size,
		VkDescriptorSetLayout uboLayout, 
		VkDescriptorSetLayout ssboLayout
	) : seDevice(device)
	{
		init(size);
		createGraphicsPipelines(uboLayout, ssboLayout);
	}
	RenderSystem::~RenderSystem() {
		destroy();
	}
	void RenderSystem::init(glm::ivec3 size) {
		createFrameBufferAttachments(size);
		createRenderPasses();
		createFrameBuffers();
	}
	void RenderSystem::destroy()
	{
	}
}