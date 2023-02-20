#pragma once
#include <scorch/vkapi/descriptors.h>

namespace ScorchEngine {
	class MaterialSystem {
	public:
		static std::unique_ptr<SEDescriptorSetLayout>& getMaterialDescriptorSetLayout() {
			return descriptorLayout;
		}
		static void createDescriptorSetLayout(SEDevice& device) {
			descriptorLayout = SEDescriptorSetLayout::Builder(device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT) // params
				.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // diffuse
				.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // emissive
				.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // normal
				.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // specular
				.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // roughness
				.addBinding(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // metallic
				.addBinding(7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // ambientOcclusion
				.addBinding(8, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // (discard) mask
				.build();
		}
		static void destroyDescriptorSetLayout() {
			descriptorLayout = nullptr;
		}
	private:
		static inline std::unique_ptr<SEDescriptorSetLayout> descriptorLayout;
	};
}