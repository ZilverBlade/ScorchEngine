#pragma once
#include  "environment_map.h"
#include <glm/gtc/constants.hpp>
#include <scorch/rendering/camera.h>

namespace ScorchEngine {
	SEEnvironmentMap::SEEnvironmentMap(SEDevice& device, SEDescriptorPool& descriptorPool, std::unique_ptr<SEDescriptorSetLayout>& descriptorSetLayout, VkDescriptorImageInfo envMapImageInfo, glm::vec2 envMapImageDimensions, bool fastGeneration)
		: seDevice(device), seDescriptorPool(descriptorPool), envMapImageInfo(envMapImageInfo), fastGeneration(fastGeneration), envMapDescriptorLayout(descriptorSetLayout){

		assert(!fastGeneration && "not supported");

		if (!envBRDFGen)
			envBRDFGen = new SEPostProcessingEffect(
			seDevice,
			{ 128, 128 },
			SEShader(SEShaderType::Fragment, "res/shaders/spirv/envbrdfgen.fsh.spv"),
			{},
			VK_FORMAT_R16G16_SFLOAT, //VK_FORMAT_R8G8_UNORM,
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
				{ envMapImageInfo },
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_IMAGE_VIEW_TYPE_CUBE,
				{},
				6,
				envPrefilteredMipLevels
			);
			envIrradianceGen = new SEPostProcessingEffect(
				seDevice,
				{ 128, 128 },
				SEShader(SEShaderType::Fragment, "res/shaders/spirv/envirrgen.fsh.spv"),
				{ envMapImageInfo },
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_IMAGE_VIEW_TYPE_CUBE,
				{},
				6,
				1
			);
		}

		SEDescriptorWriter::SEDescriptorWriter(*descriptorSetLayout, descriptorPool)
			.writeImage(0, &envPrefilteredGen->getAttachment()->getDescriptor())
			.writeImage(1, &envIrradianceGen->getAttachment()->getDescriptor())
			.writeImage(2, &envBRDFGen->getAttachment()->getDescriptor())
			.build(envMapDescriptor);
	}
	SEEnvironmentMap::~SEEnvironmentMap() {
		if (!fastGeneration) {
			delete envPrefilteredGen;
			delete envIrradianceGen;
		}
	}
	static glm::vec3 orientations[6]{
		{ 0.f, glm::half_pi<float>() , 0.f },	// right (works)
		{ 0.f, -glm::half_pi<float>() , 0.f},		// left
		{ glm::half_pi<float>(), 0.f, 0.f },	// up (works)
		{ -glm::half_pi<float>() , 0.f, 0.f },		// down 
		{ 0.f, 0.f, 0.f },						// front (works)
		{ 0.f, glm::pi<float>() , 0.f}			// back
	};
	void SEEnvironmentMap::generatePrefilteredEnvironmentMap(VkCommandBuffer commandBuffer) {
		struct EnvPrefilteredPush {
			glm::mat4 mvp;
			float roughness;
		};
		if (!fastGeneration) {
			for (int i = 0; i < 6; i++) {
				SECamera camera{};
				camera.setViewYXZ({}, orientations[i]);
				EnvPrefilteredPush push{};
				push.mvp =  camera.getView();
				for (int j = 0; j < envPrefilteredMipLevels; j++) {
					push.roughness = static_cast<float>(j) / static_cast<float>(envPrefilteredMipLevels - 1);
					envPrefilteredGen->render(commandBuffer, &push, i, j);
				}
			}
		}	
	}
	void SEEnvironmentMap::generateIrradianceMap(VkCommandBuffer commandBuffer) {
		if (!fastGeneration) {
			for (int i = 0; i < 6; i++) {
				SECamera camera{};
				camera.setViewYXZ({}, orientations[i]);
				glm::mat4 mvp = camera.getView();
				envIrradianceGen->render(commandBuffer, &mvp, i); 
			}
		}
	}
	void SEEnvironmentMap::generateEnvironmentBRDF(VkCommandBuffer commandBuffer) {
		envBRDFGen->render(commandBuffer, nullptr);
	}
}