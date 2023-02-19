#pragma once

#include "surface_material.h"
#include <scorch/systems/resource_system.h>
#include <simdjson.h>

namespace ScorchEngine {
	// copied from "res/shaders/surface_material.glsl" 
	const uint32_t SURFACE_MATERIAL_TEXTURE_DIFFUSE_BIT = 0x01;
	const uint32_t SURFACE_MATERIAL_TEXTURE_EMISSIVE_BIT = 0x02;
	const uint32_t SURFACE_MATERIAL_TEXTURE_NORMAL_BIT = 0x04;
	const uint32_t SURFACE_MATERIAL_TEXTURE_SPECULAR_BIT = 0x08;
	const uint32_t SURFACE_MATERIAL_TEXTURE_ROUGHNESS_BIT = 0x10;
	const uint32_t SURFACE_MATERIAL_TEXTURE_METALLIC_BIT = 0x20;
	const uint32_t SURFACE_MATERIAL_TEXTURE_AMBIENTOCCLUSION_BIT = 0x40;
	const uint32_t SURFACE_MATERIAL_TEXTURE_MASK_BIT = 0x80;
	const uint32_t SURFACE_MATERIAL_TEXTURE_OPACITY_BIT = 0x100;
	struct SurfaceMaterialBufferInfo {
		alignas(16)glm::vec3 diffuse;
		alignas(16)glm::vec3 emission;
		float specular;
		float roughness;
		float metallic;
		float ambientOcclusion;
		glm::vec2 uvScale;
		glm::vec2 uvOffset;
		uint32_t textureFlags;
		uint32_t shadingModelFlag;
	};

	SESurfaceMaterial::SESurfaceMaterial(SEDevice& device, SEDescriptorPool& descriptorPool, ResourceSystem* resourceSystem) :
		seDevice(device), seDescriptorPool(descriptorPool), resourceSystem(resourceSystem) {
		paramBuffer = std::make_unique<SEBuffer>(
			seDevice,
			sizeof(SurfaceMaterialBufferInfo),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		);
		paramBuffer->map();

		descriptorLayout = SEDescriptorSetLayout::Builder(seDevice)
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
		writeDescriptor();
	}
	SESurfaceMaterial::~SESurfaceMaterial() {

	}
	
	void SESurfaceMaterial::load(const std::string& filepath) {
		simdjson::dom::parser parser;
		simdjson::padded_string json = simdjson::padded_string::load(filepath);
		const auto& material = parser.parse(json);
		if (material["class"].get_string().value() != "surface") 
			throw std::runtime_error("wrong material class typed loaded as surface material!");
		if (material["model"].get_string().value() == "lit")
			this->shadingModel = SESurfaceShadingModel::Lit;
		if (material["model"].get_string().value() == "unlit")
			this->shadingModel = SESurfaceShadingModel::Unlit;
		if (material["model"].get_string().value() == "clearCoat")
			this->shadingModel = SESurfaceShadingModel::ClearCoat;

		this->doubleSided = material["doubleSided"].get_bool().value();
		
		const auto& config = material["config"];
		if (config["diffuseFactor"].error() == simdjson::error_code::SUCCESS)
			this->diffuseFactor = {
				config["diffuseFactor"].get_array().at(0).get_double().value(),
				config["diffuseFactor"].get_array().at(1).get_double().value(),
				config["diffuseFactor"].get_array().at(2).get_double().value()
			};
		if (config["emissiveFactor"].error() == simdjson::error_code::SUCCESS)
			this->emissiveFactor = {
				config["emissiveFactor"].get_array().at(0).get_double().value(),
				config["emissiveFactor"].get_array().at(1).get_double().value(),
				config["emissiveFactor"].get_array().at(2).get_double().value()
			};
		if (config["specularFactor"].error() == simdjson::error_code::SUCCESS)
			this->specularFactor = config["specularFactor"].get_double().value();
		if (config["roughnessFactor"].error() == simdjson::error_code::SUCCESS)
			this->roughnessFactor = config["roughnessFactor"].get_double().value();
		if (config["metallicFactor"].error() == simdjson::error_code::SUCCESS)
			this->metallicFactor = config["metallicFactor"].get_double().value();
		if (config["ambientOcclusionFactor"].error() == simdjson::error_code::SUCCESS)
			this->ambientOcclusionFactor = config["ambientOcclusionFactor"].get_double().value();

		if (config["diffuseTexture"].error() == simdjson::error_code::SUCCESS)
			this->diffuseTexture = std::string(config["diffuseTexture"].get_string().value());
		if (config["emissiveTexture"].error() == simdjson::error_code::SUCCESS)
			this->emissiveTexture = std::string(config["emissiveTexture"].get_string().value());
		if (config["normalTexture"].error() == simdjson::error_code::SUCCESS)
			this->normalTexture = std::string(config["normalTexture"].get_string().value());
		if (config["specularTexture"].error() == simdjson::error_code::SUCCESS)
			this->specularTexture = std::string(config["specularTexture"].get_string().value());
		if (config["roughnessTexture"].error() == simdjson::error_code::SUCCESS)
			this->roughnessTexture = std::string(config["roughnessTexture"].get_string().value());
		if (config["metallicTexture"].error() == simdjson::error_code::SUCCESS)
			this->metallicTexture = std::string(config["metallicTexture"].get_string().value());
		if (config["ambientOcclusionTexture"].error() == simdjson::error_code::SUCCESS)
			this->ambientOcclusionTexture = std::string(config["ambientOcclusionTexture"].get_string().value());
		if (config["opacityMask"].error() == simdjson::error_code::SUCCESS)
			this->ambientOcclusionTexture = std::string(config["ambientOcclusionTexture"].get_string().value());

		if (material["uvScale"].error() == simdjson::error_code::SUCCESS)
			this->uvScale = { 
				material["uvScale"].get_array().at(0).get_double().value(),
				material["uvScale"].get_array().at(1).get_double().value()
			};
		if (material["uvOffset"].error() == simdjson::error_code::SUCCESS)
			this->uvOffset = {
				material["uvOffset"].get_array().at(0).get_double().value(),
				material["uvOffset"].get_array().at(1).get_double().value()
		};

		/*
	
	"UVOffset": [ 0, 0 ],
	"Translucent": false,
	"DoubleSided": true
}
	*/
	}
	void SESurfaceMaterial::bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t setOffset) {
		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			setOffset,
			1,
			&descriptorSet,
			0,
			nullptr
		);
	}
	void SESurfaceMaterial::updateParams() {
		SurfaceMaterialBufferInfo bufferInfo{};
		bufferInfo.diffuse = this->diffuseFactor;
		bufferInfo.emission = this->emissiveFactor;
		bufferInfo.specular = this->specularFactor;
		bufferInfo.roughness = this->roughnessFactor;
		bufferInfo.metallic = this->metallicFactor;
		bufferInfo.ambientOcclusion = this->ambientOcclusionFactor;

		bufferInfo.uvOffset = this->uvOffset;
		bufferInfo.uvScale = this->uvScale;
		bufferInfo.shadingModelFlag = static_cast<uint32_t>(this->shadingModel);

		paramBuffer->writeToBuffer(&bufferInfo);
	}
	void SESurfaceMaterial::updateTextures() {
		uint32_t textureFlags = 0;

		if (diffuseTexture) textureFlags |= SURFACE_MATERIAL_TEXTURE_DIFFUSE_BIT;
		if (emissiveTexture) textureFlags |= SURFACE_MATERIAL_TEXTURE_EMISSIVE_BIT;
		if (normalTexture) textureFlags |= SURFACE_MATERIAL_TEXTURE_NORMAL_BIT;
		if (specularTexture) textureFlags |= SURFACE_MATERIAL_TEXTURE_SPECULAR_BIT;
		if (roughnessTexture) textureFlags |= SURFACE_MATERIAL_TEXTURE_ROUGHNESS_BIT;
		if (metallicTexture) textureFlags |= SURFACE_MATERIAL_TEXTURE_METALLIC_BIT;
		if (ambientOcclusionTexture) textureFlags |= SURFACE_MATERIAL_TEXTURE_AMBIENTOCCLUSION_BIT;
		if (maskTexture) textureFlags |= SURFACE_MATERIAL_TEXTURE_MASK_BIT;

		paramBuffer->writeToBuffer(&textureFlags, offsetof(SurfaceMaterialBufferInfo, textureFlags), sizeof(textureFlags));
	
		writeDescriptor();
	}
	void SESurfaceMaterial::writeDescriptor() {
		auto writer = SEDescriptorWriter::SEDescriptorWriter(*descriptorLayout, seDescriptorPool)
			.writeBuffer(0, &paramBuffer->getDescriptorInfo());

		if (diffuseTexture) writer.writeImage(1, &resourceSystem->getTexture2D({diffuseTexture, true, true})->getImageInfo());
		if (emissiveTexture) writer.writeImage(2, &resourceSystem->getTexture2D({ emissiveTexture, true, true })->getImageInfo());
		if (normalTexture) writer.writeImage(3, &resourceSystem->getTexture2D({ normalTexture, false, true })->getImageInfo());
		if (specularTexture) writer.writeImage(4, &resourceSystem->getTexture2D({ specularTexture, false, true })->getImageInfo());
		if (roughnessTexture) writer.writeImage(5, &resourceSystem->getTexture2D({ roughnessTexture, false, true })->getImageInfo());
		if (metallicTexture) writer.writeImage(6, &resourceSystem->getTexture2D({ metallicTexture, false, true })->getImageInfo());
		if (ambientOcclusionTexture) writer.writeImage(7, &resourceSystem->getTexture2D({ ambientOcclusionTexture, false, true })->getImageInfo());
		if (maskTexture) writer.writeImage(8, &resourceSystem->getTexture2D({ maskTexture, false, true })->getImageInfo());

		writer.build(descriptorSet);
	}
}
