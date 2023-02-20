#pragma once

#include <scorch/vkapi/buffer.h>
#include <scorch/vkapi/descriptors.h>
#include <scorch/utils/resid.h>
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace ScorchEngine {
	class ResourceSystem;
	enum class SESurfaceShadingModel {
		Lit = 0x01,
		Unlit = 0x02,
		ClearCoat = 0x03,
	};

	class SESurfaceMaterial {
	public:
		SESurfaceMaterial(SEDevice& device, SEDescriptorPool& descriptorPool, ResourceSystem* resourceSystem);
		~SESurfaceMaterial();

		SESurfaceMaterial(const SESurfaceMaterial&) = delete;
		SESurfaceMaterial operator=(const SESurfaceMaterial&) = delete;

		void load(const std::string& filepath);
		void bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t descriptorOffset);
		void updateParams();
		void updateTextures();

		SESurfaceShadingModel shadingModel = SESurfaceShadingModel::Lit;

		ResourceID diffuseTexture{};
		ResourceID emissiveTexture{};
		ResourceID normalTexture{};
		ResourceID specularTexture{};
		ResourceID roughnessTexture{};
		ResourceID metallicTexture{};
		ResourceID ambientOcclusionTexture{};
		ResourceID maskTexture{};

		glm::vec3 diffuseFactor{ 1.f };
		glm::vec3 emissiveFactor{ 0.f };
		float specularFactor = 1.0f;
		float roughnessFactor = 1.0f;
		float metallicFactor = 1.0f;
		float ambientOcclusionFactor = 1.0f;

	//	float clearCoatFactor = 1.0f;
	//	float clearCoatRoughnessFactor = 0.0f;

		glm::vec2 uvScale{ 1.f };
		glm::vec2 uvOffset{ 0.f };

		bool doubleSided = false;
		bool translucent = false;
	private:
		void writeDescriptor();

		SEDevice& seDevice;
		SEDescriptorPool& seDescriptorPool;

		std::unique_ptr<SEBuffer> paramBuffer{};

		VkDescriptorSet descriptorSet{};
		ResourceSystem* resourceSystem;
	};
}
