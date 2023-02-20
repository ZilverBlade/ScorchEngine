#pragma once

#include "model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace ScorchEngine {
	bool SEModel::Builder::loadModel(const std::string& filepath) {
		Assimp::Importer importer;
		uint32_t processFlags =
			aiProcess_Triangulate |
			aiProcess_OptimizeMeshes |
			aiProcess_JoinIdenticalVertices |
			//aiProcess_MakeLeftHanded |
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_PreTransformVertices | 
			aiProcess_FlipUVs;

		const aiScene* scene = importer.ReadFile(filepath, processFlags);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			SELOG_ERR("Error importing file '%s': %s", filepath, importer.GetErrorString());
			return false;
		}

		format = filepath.substr(filepath.find_last_of(".") + 1);

		processNode(scene->mRootNode, scene);
	}
	void SEModel::Builder::processNode(aiNode* node, const aiScene* scene) {
		// process all the node's meshes (if any)
		for (uint32_t i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			loadSubmesh(mesh, scene);
		}
		// then do the same for each of its children
		for (uint32_t i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);
		}
	}
	void SEModel::Builder::loadSubmesh(aiMesh* mesh, const aiScene* scene) {
		float factor{ 1.0f };
		const uint32_t index = mesh->mMaterialIndex;
		const aiMaterial* material = scene->mMaterials[index];
		std::string materialSlot = material->GetName().C_Str();
		if (this->format == "fbx") factor *= 0.01f; // fbx unit is in cm for some reason
		if (this->format == "gltf") materialSlot = "material_" + std::to_string(index); // cant reliably get material slot name with gltf

		if (submeshes.find(materialSlot) == submeshes.end()) {
			submeshes[materialSlot] = {};
		}
		Builder::Submesh& submesh = submeshes[materialSlot];

		for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
			submesh.vertexPositions.push_back({
				factor * mesh->mVertices[i].x,
				factor * mesh->mVertices[i].z,
				factor * mesh->mVertices[i].y
			});
			submesh.vertexUVs.push_back({
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			});
			submesh.vertexNormals.push_back({
				mesh->mNormals[i].x,
				mesh->mNormals[i].z,
				mesh->mNormals[i].y
			});
			submesh.vertexTangents.push_back({
				mesh->mNormals[i].x,
				mesh->mNormals[i].z,
				mesh->mNormals[i].y
			});
		}

		submesh.indices.reserve(mesh->mNumFaces * 3u);
		for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++)
				submesh.indices.push_back(face.mIndices[j]);
		}
	}
	
	SEModel::SEModel(SEDevice& device, const SEModel::Builder& builder) : seDevice(device) {
		for (const auto& [slot, data] : builder.submeshes) {
			submeshes[slot] = {};
			Submesh& submesh = submeshes[slot];

			submesh.vertexPositionBuffer = createBuffer(data.vertexPositions);
			submesh.vertexUVBuffer = createBuffer(data.vertexUVs);
			submesh.vertexNormalBuffer = createBuffer(data.vertexNormals);
			submesh.vertexTangentsBuffer = createBuffer(data.vertexTangents);
			submesh.indexBuffer = createBuffer(data.indices);

			submesh.vertexCount = submesh.vertexPositionBuffer->getInstanceCount();
			submesh.indexCount = submesh.indexBuffer->getInstanceCount();
		}
	}
	SEModel::~SEModel() {
		
	}

	void SEModel::bind(VkCommandBuffer commandBuffer, const std::string& submeshName) {
		auto iter = submeshes.find(submeshName);
		if (iter == submeshes.end()) {
			throw std::runtime_error("submesh '" + submeshName + "'not found!");
		}
		boundSubmesh = submeshName;
		Submesh& submesh = (*iter).second;

		VkBuffer vertexBuffers[4]{
			submesh.vertexPositionBuffer->getBuffer(),
			submesh.vertexUVBuffer->getBuffer(),
			submesh.vertexNormalBuffer->getBuffer(),
			submesh.vertexTangentsBuffer->getBuffer()
		};
		VkDeviceSize offsets[4]{
			0,0,0,0
		};
		vkCmdBindVertexBuffers(commandBuffer, 0, 4, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, submesh.indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void SEModel::draw(VkCommandBuffer commandBuffer) {
		vkCmdDrawIndexed(commandBuffer, submeshes[boundSubmesh].indexCount, 1, 0, 0, 0);
	}

	std::vector<std::string> SEModel::getSubmeshes() {
		std::vector<std::string> strings;
		for (const auto& [slot, submesh] : submeshes) {
			strings.push_back(slot);
		}
		return strings;
	}

	template<typename T>
	inline std::unique_ptr<SEBuffer> SEModel::createBuffer(const std::vector<T>& data) {
		std::unique_ptr<SEBuffer> stagingBuffer = std::make_unique<SEBuffer>(
			seDevice,
			sizeof(T),
			data.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
		stagingBuffer->map();
		stagingBuffer->writeToBuffer(data.data());

		std::unique_ptr<SEBuffer> dstBuffer = std::make_unique<SEBuffer>(
			seDevice,
			sizeof(T),
			data.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		VkCommandBuffer commandBuffer = seDevice.beginSingleTimeCommands();
		seDevice.copyBuffer(commandBuffer, stagingBuffer->getBuffer(), dstBuffer->getBuffer(), stagingBuffer->getBufferSize());
		seDevice.endSingleTimeCommands(commandBuffer);
		return dstBuffer;
	}
}
