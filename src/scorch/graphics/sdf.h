#pragma once
#pragma once

#include <scorch/vkapi/buffer.h>
#include <scorch/utils/resid.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <array>
#include <scorch/vkapi/empty_texture.h>
#include <scorch/vkapi/descriptors.h>

extern "C" {
	struct aiNode;
	struct aiScene;
	struct aiMesh;
	struct aiMaterial;
}
namespace ScorchEngine {
	class SEVoxelSDF {
	public:
		struct Builder {
			Builder& setVertices(std::vector<glm::vec3> vertices);
			Builder& setTriangles(std::vector<std::array<uint32_t, 3>> triangles);
			Builder& setResolution(glm::ivec3 dim);
			void build();
		private:
			int findClosestTriangle(glm::vec3 point, float& outSignedDist);
			float signedDistanceSquareToTriangle(glm::vec3 point, std::array<uint32_t, 3> triangle);

			std::vector<glm::vec3> vertices;
			std::vector<std::array<uint32_t, 3>> triangles;
			glm::ivec3 resolution;	
			glm::vec3 boundsHalfExtent;
			glm::vec3 boundsCenter;
			std::vector<float> distanceFieldPoints;
			friend class SEVoxelSDF;
		};

		SEVoxelSDF(SEDevice& device, SEDescriptorPool& descriptorPool, const SEVoxelSDF::Builder& builder);
		~SEVoxelSDF();

		SEVoxelSDF(const SEVoxelSDF&) = delete;
		SEVoxelSDF& operator=(const SEVoxelSDF&) = delete;

		VkDescriptorSetLayout getDescriptorSetLayout() {
			return descriptorSetLayout->getDescriptorSetLayout();
		}
		VkDescriptorSet getDescriptorSet() {
			return distanceFieldDescriptor;
		}
		glm::vec3 getHalfExtent() {
			return halfExtent;
		}
		glm::ivec3 getResolution() {
			return distanceFieldTexture->getDimensions();
		}
	private:
		void createVoxel(const SEVoxelSDF::Builder& builder);
		void createDescriptorSetLayout();
		void createDescriptor();


		SEDevice& seDevice;
		SEDescriptorPool& seDescriptorPool;
		std::unique_ptr<SEDescriptorSetLayout> descriptorSetLayout;

		glm::vec3 halfExtent;

		VkDescriptorSet distanceFieldDescriptor=nullptr;
		SEEmptyTexture* distanceFieldTexture;
	};
}
