#pragma once
#include  "sky_light.h"

namespace ScorchEngine {
	SESkyLight::SESkyLight(SEDevice& device, SEDescriptorPool& descriptorPool, std::unique_ptr<SEDescriptorSetLayout>& descriptorSetLayout, VkDescriptorImageInfo skyLightImageInfo, glm::vec2 skyLightImageDimensions, bool fastGeneration)
		: seDevice(device), seDescriptorPool(descriptorPool), skyLightImageInfo(skyLightImageInfo), fastGeneration(fastGeneration), skyLightDescriptorLayout(descriptorSetLayout){

		assert(!fastGeneration && "not supported");

		if (!envBRDFGen)
			envBRDFGen = new SEPostProcessingEffect(
			seDevice,
			{ 128, 128 },
			SEShader(SEShaderType::Fragment, "res/shaders/envbrdfgen.fsh"),
			descriptorPool,
			{},
			VK_FORMAT_R8G8_UNORM,
			VK_IMAGE_VIEW_TYPE_2D
		);
		if (!envBRDFGenerated) {
			VkCommandBuffer commandBuffer = seDevice.beginSingleTimeCommands();
			generateEnvironmentBRDF(commandBuffer);
			seDevice.endSingleTimeCommands(commandBuffer);
		}
		envPrefilteredMipLevels = std::min(static_cast<uint32_t>(std::floor(std::log2(skyLightImageDimensions.x))) - 2, 5U); // limit to suit 128x128 reflection texture, any lower looks horrible
		if (!fastGeneration) {
			envPrefilteredGen = new SEPostProcessingEffect(
				seDevice,
				skyLightImageDimensions,
				SEShader(SEShaderType::Fragment, "res/shaders/envprefiltergen.fsh"),
				descriptorPool,
				{},
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_IMAGE_VIEW_TYPE_CUBE,
				6,
				envPrefilteredMipLevels
			);
			envIrradianceGen = new SEPostProcessingEffect(
				seDevice,
				{ 128, 128 },
				SEShader(SEShaderType::Fragment, "res/shaders/envirrgen.fsh"),
				descriptorPool,
				{},
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_IMAGE_VIEW_TYPE_CUBE,
				6,
				1
			);
		}

	}
	SESkyLight::~SESkyLight() {
		if (!fastGeneration) {
			delete envPrefilteredGen;
			delete envIrradianceGen;
		}
	}
	void SESkyLight::generatePrefilteredEnvironmentMap(VkCommandBuffer commandBuffer) {
		struct EnvPrefilteredPush {
			uint32_t face;
			uint32_t mip;
		};
		if (!fastGeneration) {
			for (int i = 0; i < 6; i++) {
				for (int j = 0; j < envPrefilteredMipLevels; j++) {
					envPrefilteredGen->render(commandBuffer, nullptr, i, j);
				}
			}
		}	
	}
	void SESkyLight::generateIrradianceMap(VkCommandBuffer commandBuffer) {
		if (!fastGeneration) {
			for (int i = 0; i < 6; i++) {
				envPrefilteredGen->render(commandBuffer, nullptr, i);
			}
		}
	}
	void SESkyLight::generateEnvironmentBRDF(VkCommandBuffer commandBuffer) {
		envBRDFGen->render(commandBuffer, nullptr);
	}
}