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
		if (this->format == "fbx") factor *= 0.01f; // fbx unit is in cm for some reason

		for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
			vertexPositions.push_back({
				factor * mesh->mVertices[i].x,
				factor * mesh->mVertices[i].z,
				factor * mesh->mVertices[i].y
			});
			vertexUVs.push_back({
				mesh->mTextureCoords[i][0].x,
				mesh->mTextureCoords[i][0].y
			});
			vertexNormals.push_back({
				mesh->mNormals[i].x,
				mesh->mNormals[i].z,
				mesh->mNormals[i].y
			});
			vertexTangents.push_back({
				mesh->mNormals[i].x,
				mesh->mNormals[i].z,
				mesh->mNormals[i].y
			});
		}

		indices.reserve(mesh->mNumFaces * 3u);
		for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
	}
	
	SEModel::SEModel(SEDevice& device, const SEModel::Builder& builder) : seDevice(device) {
		vertexPositionBuffer = createBuffer(builder.vertexPositions);
		vertexUVBuffer = createBuffer(builder.vertexUVs);
		vertexNormalBuffer = createBuffer(builder.vertexNormals);
		vertexTangentsBuffer = createBuffer(builder.vertexTangents);
		indexBuffer = createBuffer(builder.indices);

		vertexCount = vertexPositionBuffer->getInstanceCount();
		indexCount = indexBuffer->getInstanceCount();
	}
	SEModel::~SEModel() {
		
	}

	void SEModel::bind(VkCommandBuffer commandBuffer) {
		VkBuffer vertexBuffers[4]{
			vertexPositionBuffer->getBuffer(),
			vertexUVBuffer->getBuffer(),
			vertexNormalBuffer->getBuffer(),
			vertexTangentsBuffer->getBuffer()
		};
		VkDeviceSize offsets[4]{
			0,0,0,0
		};
		vkCmdBindVertexBuffers(commandBuffer, 0, 4, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);	
	}

	void SEModel::draw(VkCommandBuffer commandBuffer) {
		vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
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

		stagingBuffer->writeToBuffer(data.data());

		std::unique_ptr<SEBuffer> dstBuffer = std::make_unique<SEBuffer>(
			seDevice,
			sizeof(T),
			data.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		seDevice.copyBuffer(stagingBuffer->getBuffer(), dstBuffer->getBuffer(), stagingBuffer->getBufferSize());

		return std::move(stagingBuffer);
	}
}
