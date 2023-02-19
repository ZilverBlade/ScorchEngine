#include <scorch/apps/vulkan_base.h>
namespace ScorchEngine::Apps {
	VulkanBaseApp::VulkanBaseApp(const char* name) :
		App(name), 
		seRenderer(new SERenderer(seDevice)), 
		seSwapChain(new SESwapChain(seDevice, seWindow, seWindow.getExtent())) 
	{
		uint32_t imageCount = seSwapChain->getImageCount();
		globalUBODescriptorLayout = SEDescriptorSetLayout::Builder(seDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();
		sceneSSBODescriptorLayout = SEDescriptorSetLayout::Builder(seDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		inFlightPool = SEDescriptorPool::Builder(seDevice)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, imageCount)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, imageCount)
			.setMaxSets(imageCount * 2)
			.build();

		staticPool = SEDescriptorPool::Builder(seDevice)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4096)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4096)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096)
			.setMaxSets(4096 * 3)
			.build();

		for (int i = 0; i < seSwapChain->getImageCount(); i++) {
			InFlightRenderData data{};
			data.uboBuffer = std::make_unique<SEBuffer>(
				seDevice,
				sizeof(GlobalUBO),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			data.uboBuffer->map();
			SEDescriptorWriter(*globalUBODescriptorLayout, *inFlightPool)
				.writeBuffer(0, &data.uboBuffer->getDescriptorInfo())
				.build(data.uboDescriptorSet);

			data.sceneSSBO = std::make_unique<SceneSSBO>(); // allocate this on the heap because it can get f***ing massive
			data.ssboBuffer = std::make_unique<SEBuffer>(
				seDevice,
				sizeof(SceneSSBO),
				1,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			data.ssboBuffer->map();
			SEDescriptorWriter(*sceneSSBODescriptorLayout, *inFlightPool)
				.writeBuffer(0, &data.ssboBuffer->getDescriptorInfo())
				.build(data.ssboDescriptorSet);
			renderData.push_back(std::move(data));
		}
	}
	VulkanBaseApp::~VulkanBaseApp() {
		delete seRenderer;
		delete seSwapChain;
		renderData.clear();
	}
	void VulkanBaseApp::run() {
		
	}
}