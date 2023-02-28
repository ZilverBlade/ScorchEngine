#pragma once
#include <scorch/vk.h>
#include <scorch/systems/post_fx/post_processing_fx.h>
#include <scorch/graphics/texture_cube.h>

namespace ScorchEngine {
	class SEEnvironmentMap {
	public:
		SEEnvironmentMap(SEDevice& device, SEDescriptorPool& descriptorPool, std::unique_ptr<SEDescriptorSetLayout>& descriptorSetLayout, VkDescriptorImageInfo envMapImageInfo, glm::vec2 envMapImageDimensions, bool fastGeneration);
		~SEEnvironmentMap();

		SEEnvironmentMap(const SEEnvironmentMap&) = delete;
		SEEnvironmentMap& operator=(const SEEnvironmentMap&) = delete;

		void generatePrefilteredEnvironmentMap(VkCommandBuffer commandBuffer);
		void generateIrradianceMap(VkCommandBuffer commandBuffer);
		static void generateEnvironmentBRDF(VkCommandBuffer commandBuffer); // env brdf only needs to be generated once at the start of the application
		VkDescriptorSet getEnvironmentMapDescriptor() {
			return envMapDescriptor;
		}
	private:
		SEDevice& seDevice;
		SEDescriptorPool& seDescriptorPool;
		std::unique_ptr<SEDescriptorSetLayout>& envMapDescriptorLayout;

		VkDescriptorSet envMapDescriptor{};

		VkDescriptorImageInfo  envMapImageInfo{}; // not a SETextureCube* since I plan on adding dynamic reflection cubemapping which means this could come from a render pass

		static inline SEPostProcessingEffect* envBRDFGen{};
		static inline bool envBRDFGenerated = false;

		uint32_t envPrefilteredMipLevels{};
		SEPostProcessingEffect* envPrefilteredGen{};
		SEPostProcessingEffect* envIrradianceGen{};

		SEPostProcessingEffect* envIrradianceGenFastPass0{}; // size / 2 horizontal
		SEPostProcessingEffect* envIrradianceGenFastPass1{}; // size / 2 vertical
		SEPostProcessingEffect* envIrradianceGenFastPass2{}; // size / 4 horizontal
		SEPostProcessingEffect* envIrradianceGenFastPass3{}; // size / 4 vertical
		SEPostProcessingEffect* envIrradianceGenFastPass4{}; // size / 8 horizontal
		SEPostProcessingEffect* envIrradianceGenFastPass5{}; // size / 8 vertical

		bool fastGeneration = false;
	};
}