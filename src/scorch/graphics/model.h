#pragma once

#include <scorch/vkapi/buffer.h>
#include <scorch/utils/resid.h>
#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <unordered_map>
#include <assimp/Importer.hpp>
extern "C" {
	struct aiNode;
	struct aiScene;
	struct aiMesh;
	struct aiMaterial;
}
namespace ScorchEngine {
	class ResourceSystem;
	class SESurfaceMaterial;
	class SEVoxelSdf;
	class SEDescriptorPool;
	class SEModel {
	public:
		struct Submesh {
			std::unique_ptr<SEBuffer> vertexPositionBuffer;
			std::unique_ptr<SEBuffer> vertexUVBuffer;
			std::unique_ptr<SEBuffer> vertexNormalBuffer;
			std::unique_ptr<SEBuffer> vertexTangentsBuffer;
			std::unique_ptr<SEBuffer> indexBuffer;

			uint32_t vertexCount{};
			uint32_t indexCount{};
		};
		struct Builder {
			struct Submesh {
				std::vector<glm::vec3> vertexPositions{};
				std::vector<glm::vec2> vertexUVs{};
				std::vector<glm::vec3> vertexNormals{};
				std::vector<glm::vec3> vertexTangents{};
				std::vector<uint32_t> indices{};
			};
			bool loadModel(const std::string& filepath);
		private:
			bool decodeJsonProperties(const std::string& filepath);
			void processNode(aiNode* node, const aiScene* scene);
			void loadSubmesh(aiMesh* mesh, const aiScene* scene);
			std::shared_ptr<std::unordered_map<std::string, ResourceID>> loadMaterials(SEDevice& device, ResourceSystem* resourceSystem);

		private:
			float importScale = 1.0f;
			Assimp::Importer importer;
			const aiScene* scene;
			std::unordered_map<std::string, Submesh> submeshes{};
			std::unordered_map<std::string, uint32_t> materialInfos;
			std::string format;
			std::string filePath;
			std::string modelPath;
			glm::ivec3 sdfResolution;
			float sdfConservativeFactor;
			friend class SEModel;
		};

		SEModel(SEDevice& device, SEDescriptorPool& descriptorPool, const SEModel::Builder& builder);
		~SEModel();

		SEModel(const SEModel &) = delete;
		SEModel &operator=(const SEModel &) = delete;

		void bind(VkCommandBuffer commandBuffer, const std::string& submeshName);
		void draw(VkCommandBuffer commandBuffer);
		
		std::vector<std::string> getSubmeshes();

		SEVoxelSdf& getSdf();
	private:
		void createSubmeshes(const SEModel::Builder& builder);
		void createSdf(SEDescriptorPool& descriptorPool, const SEModel::Builder& builder);

		template <typename T>
		std::unique_ptr<SEBuffer> createBuffer(const std::vector<T> & data);

		SEDevice &seDevice;

		SEVoxelSdf* sdf;

		std::string boundSubmesh{};
		std::unordered_map<std::string, Submesh> submeshes{};
	};
}
