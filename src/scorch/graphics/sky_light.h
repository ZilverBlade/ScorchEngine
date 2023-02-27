#pragma once
#include <scorch/vk.h>
#include <scorch/systems/post_fx/post_processing_fx.h>
#include <scorch/graphics/texture_cube.h>

namespace ScorchEngine {
	class SESkyLight {
	public:
		SESkyLight(SEDevice& device, SEDescriptorPool& descriptorPool, std::unique_ptr<SEDescriptorSetLayout>& descriptorSetLayout, VkDescriptorImageInfo skyLightImageInfo, glm::vec2 skyLightImageDimensions, bool fastGeneration);
		~SESkyLight();

		SESkyLight(const SESkyLight&) = delete;
		SESkyLight& operator=(const SESkyLight&) = delete;

		void generatePrefilteredEnvironmentMap(VkCommandBuffer commandBuffer);
		void generateIrradianceMap(VkCommandBuffer commandBuffer);
		static void generateEnvironmentBRDF(VkCommandBuffer commandBuffer); // env brdf only needs to be generated once at the start of the application
		VkDescriptorSet getEnvironmentMapDescriptor() {
			return skyLightDescriptor;
		}
	private:
		SEDevice& seDevice;
		SEDescriptorPool& seDescriptorPool;
		std::unique_ptr<SEDescriptorSetLayout>& skyLightDescriptorLayout;

		VkDescriptorSet skyLightDescriptor{};

		VkDescriptorImageInfo skyLightImageInfo{}; // not a SETextureCube* since I plan on adding dynamic reflection cubemapping which means this could come from a render pass

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