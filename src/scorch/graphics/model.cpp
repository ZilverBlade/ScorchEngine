#pragma once

#include "model.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <scorch/systems/resource_system.h>
#include <scorch/graphics/sdf.h>
#include <filesystem>

namespace ScorchEngine {
	bool SEModel::Builder::loadModel(const std::string& filepath) {
		filePath = filepath;
		uint32_t processFlags =
			aiProcess_Triangulate |
			aiProcess_OptimizeMeshes |
			aiProcess_JoinIdenticalVertices |
			//aiProcess_MakeLeftHanded |
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_PreTransformVertices | 
			aiProcess_FlipUVs;

		modelPath = filepath.substr(0, filepath.find_last_of("/"));
		scene = importer.ReadFile(filepath, processFlags);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			SELOG_ERR("Error importing file '%s': %s", filepath.c_str(), importer.GetErrorString());
			return false;
		}

		format = filepath.substr(filepath.find_last_of(".") + 1);

		processNode(scene->mRootNode, scene);
		return true;
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
		aiMaterial* material = scene->mMaterials[index];
		std::string materialSlot = material->GetName().C_Str();
		if (this->format == "fbx") factor *= 0.01f; // fbx unit is in cm for some reason
		if (this->format == "gltf") materialSlot = "material_" + std::to_string(index); // cant reliably get material slot name with gltf

		if (submeshes.find(materialSlot) == submeshes.end()) {
			submeshes[materialSlot] = {};
			materialInfos[materialSlot] = index;
		}
		Builder::Submesh& submesh = submeshes[materialSlot];

		for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
			submesh.vertexPositions.push_back({
				factor * mesh->mVertices[i].x,
				factor * mesh->mVertices[i].y,
				factor * mesh->mVertices[i].z
			});
			submesh.vertexUVs.push_back({
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			});
			submesh.vertexNormals.push_back({
				mesh->mNormals[i].x,
				mesh->mNormals[i].y,
				mesh->mNormals[i].z
			});
			submesh.vertexTangents.push_back({
				mesh->mNormals[i].x,
				mesh->mNormals[i].y,
				mesh->mNormals[i].z
			});
		}

		submesh.indices.reserve(mesh->mNumFaces * 3u);
		for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++)
				submesh.indices.push_back(face.mIndices[j]);
		}
	}

	void SEModel::Builder::setSDFQuality(glm::ivec3 resolution) {
		sdfResolution = resolution;
	}

	std::shared_ptr<std::unordered_map<std::string, ResourceID>>
		SEModel::Builder::loadMaterials(SEDevice& device, ResourceSystem* resourceSystem
	) {
		auto outMaterials = std::make_shared<std::unordered_map<std::string, ResourceID>>();

		assert(scene && "aiScene must be loaded!");
		for (auto& [slot, index] : materialInfos) {
			aiMaterial* material = scene->mMaterials[index];
			// Diffuse
			ResourceID diffuseID = ResourceID();
			if (aiString diffuseTex{}; aiGetMaterialTexture(material, aiTextureType_DIFFUSE, 0, &diffuseTex) == AI_SUCCESS) {
				std::string texture = modelPath + "/" + diffuseTex.C_Str(); // no embedded textures lmfao
				if (std::filesystem::exists(texture)) {
					//std::string out = outputLocation + "/" + diffuseTex.C_Str();
					//std::filesystem::copy(texture, out);
					diffuseID = resourceSystem->loadTexture2D(texture, true, true).id;
				}
			}

			// Normal
			ResourceID normalID = ResourceID();
			if (aiString normalTex{}; aiGetMaterialTexture(material, aiTextureType_NORMALS, 0, &normalTex) == AI_SUCCESS) {
				std::string texture = modelPath + "/" + normalTex.C_Str(); // no embedded textures lmfao
				if (std::filesystem::exists(texture)) {
					//std::string out = outputLocation + "/" + normalTex.C_Str();
					//std::filesystem::copy(texture, out);
					normalID = resourceSystem->loadTexture2D(texture, true, true).id;
				}
			}

			// Roughness
			ResourceID roughnessID = ResourceID();
			if (aiString roughnessTex{}; aiGetMaterialTexture(material, aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughnessTex) == AI_SUCCESS) {
				std::string texture = modelPath + "/" + roughnessTex.C_Str(); // no embedded textures lmfao
				if (std::filesystem::exists(texture)) {
					//std::string out = outputLocation + "/" + roughnessTex.C_Str();
					//std::filesystem::copy(texture, out);
					roughnessID = resourceSystem->loadTexture2D(texture, true, true).id;
				}
			}

			// Metallic
			ResourceID metallicID = ResourceID();
			if (aiString metallicTex{}; aiGetMaterialTexture(material, aiTextureType_METALNESS, 0, &metallicTex) == AI_SUCCESS) {
				std::string texture = modelPath + "/" + metallicTex.C_Str(); // no embedded textures lmfao
				if (std::filesystem::exists(texture)) {
					//std::string out = outputLocation + "/" + metallicTex.C_Str();
					//std::filesystem::copy(texture, out);
					metallicID = resourceSystem->loadTexture2D(texture, true, true).id;
				}
			}

			// Alpha Mask
			ResourceID maskID = ResourceID();
			if (aiString maskTex{}; aiGetMaterialTexture(material, aiTextureType_OPACITY, 0, &maskTex) == AI_SUCCESS) {
				std::string texture = modelPath + "/" + maskTex.C_Str(); // no embedded textures lmfao
				if (std::filesystem::exists(texture)) {
					//std::string out = outputLocation + "/" + metallicTex.C_Str();
					//std::filesystem::copy(texture, out);
					maskID = resourceSystem->loadTexture2D(texture, true, true).id;
				}
			}

			// Create material

			aiColor4D color{ 1.f, 1.f, 1.f, 1.f };
			aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &color);
			float specular{ 0.5f };
			aiGetMaterialFloat(material, AI_MATKEY_SPECULAR_FACTOR, &specular);
			float roughness{ 0.5f };
			aiGetMaterialFloat(material, AI_MATKEY_ROUGHNESS_FACTOR, &roughness);
			float metallic{ 0.0f };
			aiGetMaterialFloat(material, AI_MATKEY_METALLIC_FACTOR, &metallic);

			SESurfaceMaterial* sfMaterial = new SESurfaceMaterial(device, resourceSystem->getDescriptorPool(), resourceSystem);
			sfMaterial->translucent = false;
			sfMaterial->doubleSided = false;
			sfMaterial->shadingModel = SESurfaceShadingModel::Lit;
			sfMaterial->diffuseFactor = (glm::vec3&)color;
			sfMaterial->specularFactor = specular;
			sfMaterial->roughnessFactor = roughness;
			sfMaterial->metallicFactor = metallic;

			sfMaterial->diffuseTexture = diffuseID;
			sfMaterial->normalTexture = normalID;
			sfMaterial->roughnessTexture = roughnessID;
			sfMaterial->metallicTexture = metallicID;
			sfMaterial->maskTexture = maskID;
			
			sfMaterial->updateParams();
			sfMaterial->updateTextures();
			ResourceID sfMaterialID = modelPath.substr(modelPath.find_last_of("/") + 1, modelPath.find_last_of(".")) + "_" + slot;
			assert(resourceSystem->insertSurfaceMaterial(sfMaterialID, sfMaterial)); // pray that this works because im too lazy to add a fix in case it fails
			(*outMaterials)[slot] = sfMaterialID;
		}
		return outMaterials;
	}
	
	SEModel::SEModel(SEDevice& device, SEDescriptorPool& descriptorPool, const SEModel::Builder& builder) 
		: seDevice(device) {
		createSubmeshes(builder);
		createSDF(descriptorPool, builder);
	}
	SEModel::~SEModel() {
		delete sdf;
	}

	void SEModel::bind(VkCommandBuffer commandBuffer, const std::string& submeshName) {
		auto iter = submeshes.find(submeshName);
		if (iter == submeshes.end()) {
			throw std::runtime_error("submesh '" + submeshName + "' not found!");
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

	SEVoxelSDF& SEModel::getSDF() {
		return *sdf;
	}

	void SEModel::createSubmeshes(const SEModel::Builder& builder) {
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

	void SEModel::createSDF(SEDescriptorPool& descriptorPool, const SEModel::Builder& builder) {
		SELOG_INF("Creating SDF for %s [%ix%ix%i]", builder.filePath.c_str(),
			builder.sdfResolution.x, builder.sdfResolution.y, builder.sdfResolution.z);
		std::vector<glm::vec3> vertices;
		std::vector<std::array<uint32_t, 3>> triangles;
		for (const auto& [slot, data] : builder.submeshes) {
			{
				size_t offset = vertices.size();
				vertices.resize(vertices.size() + data.vertexPositions.size());
				std::copy(data.vertexPositions.begin(), data.vertexPositions.end(), vertices.begin() + offset);
			} 
			{
				size_t offset = triangles.size();
				triangles.resize(triangles.size() + data.indices.size() / 3);
				memcpy(
					reinterpret_cast<uint8_t*>(triangles.data()) + offset * 12,
					data.indices.data(),
					data.indices.size() * sizeof(uint32_t)
				);
			}
		}
		SEVoxelSDF::Builder sdfBuilder;
		sdfBuilder	.setVertices(vertices)
					.setTriangles(triangles)
					.setResolution(builder.sdfResolution)
					.build();
		sdf = new SEVoxelSDF(seDevice, descriptorPool, sdfBuilder);
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
