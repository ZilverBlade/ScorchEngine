#pragma once
#include  "environment_map.h"

namespace ScorchEngine {
	SEEnvironmentMap::SEEnvironmentMap(SEDevice& device, SEDescriptorPool& descriptorPool, std::unique_ptr<SEDescriptorSetLayout>& descriptorSetLayout, VkDescriptorImageInfo envMapImageInfo, glm::vec2 envMapImageDimensions, bool fastGeneration)
		: seDevice(device), seDescriptorPool(descriptorPool), envMapImageInfo(envMapImageInfo), fastGeneration(fastGeneration), envMapDescriptorLayout(descriptorSetLayout){

		assert(!fastGeneration && "not supported");

		if (!envBRDFGen)
			envBRDFGen = new SEPostProcessingEffect(
			seDevice,
			{ 128, 128 },
			SEShader(SEShaderType::Fragment, "res/shaders/spirv/envbrdfgen.fsh.spv"),
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
		envPrefilteredMipLevels = std::min(static_cast<uint32_t>(std::floor(std::log2(envMapImageDimensions.x))) - 2, 5U); // limit to suit 128x128 reflection texture, any lower looks horrible
		if (!fastGeneration) {
			envPrefilteredGen = new SEPostProcessingEffect(
				seDevice,
				envMapImageDimensions,
				SEShader(SEShaderType::Fragment, "res/shaders/spirv/envprefiltergen.fsh.spv"),
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
				SEShader(SEShaderType::Fragment, "res/shaders/spirv/envirrgen.fsh.spv"),
				descriptorPool,
				{},
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_IMAGE_VIEW_TYPE_CUBE,
				6,
				1
			);
		}

	}
	SEEnvironmentMap::~SEEnvironmentMap() {
		if (!fastGeneration) {
			delete envPrefilteredGen;
			delete envIrradianceGen;
		}
	}
	void SEEnvironmentMap::generatePrefilteredEnvironmentMap(VkCommandBuffer commandBuffer) {
		struct EnvPrefilteredPush {
			uint32_t face;
			float roughness;
		};
		if (!fastGeneration) {
			for (int i = 0; i < 6; i++) {
				for (int j = 0; j < envPrefilteredMipLevels; j++) {
					EnvPrefilteredPush push{};
					push.face = i;
					push.roughness = static_cast<float>(j) / static_cast<float>(envPrefilteredMipLevels - 1);
					envPrefilteredGen->render(commandBuffer, nullptr, i, j);
				}
			}
		}	
	}
	void SEEnvironmentMap::generateIrradianceMap(VkCommandBuffer commandBuffer) {
		if (!fastGeneration) {
			for (int i = 0; i < 6; i++) {
				envPrefilteredGen->render(commandBuffer, &i, i); // face ID for push constant
			}
		}
	}
	void SEEnvironmentMap::generateEnvironmentBRDF(VkCommandBuffer commandBuffer) {
		envBRDFGen->render(commandBuffer, nullptr);
	}
}