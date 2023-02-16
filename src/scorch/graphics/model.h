#pragma once

#include <scorch/vkapi/buffer.h>
#include <glm/glm.hpp>

#include <memory>
#include <vector>
extern "C" {
	struct aiNode;
	struct aiScene;
	struct aiMesh;
}
namespace ScorchEngine {
	class SEModel {
	public:
		struct Builder {
			std::vector<glm::vec3> vertexPositions{};
			std::vector<glm::vec2> vertexUVs{};
			std::vector<glm::vec3> vertexNormals{};
			std::vector<glm::vec3> vertexTangents{};
			std::vector<uint32_t> indices{};

			bool loadModel(const std::string &filepath);
			void processNode(aiNode* node, const aiScene* scene);
			void loadSubmesh(aiMesh* mesh, const aiScene* scene);
			std::string format;
		};

		SEModel(SEDevice& device, const SEModel::Builder& builder);
		~SEModel();

		SEModel(const SEModel &) = delete;
		SEModel &operator=(const SEModel &) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	private:
		template <typename T>
		std::unique_ptr<SEBuffer> createBuffer(const std::vector<T> & data);

		SEDevice &seDevice;

		std::unique_ptr<SEBuffer> vertexPositionBuffer;
		std::unique_ptr<SEBuffer> vertexUVBuffer;
		std::unique_ptr<SEBuffer> vertexNormalBuffer;
		std::unique_ptr<SEBuffer> vertexTangentsBuffer;
		std::unique_ptr<SEBuffer> indexBuffer;

		uint32_t vertexCount{};
		uint32_t indexCount{};
	};
}
