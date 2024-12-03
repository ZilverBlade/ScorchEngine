#include "sdf.h"

namespace ScorchEngine {
	SEVoxelSDF::Builder& SEVoxelSDF::Builder::setVertices(std::vector<glm::vec3> vertices) {
		this->vertices = vertices;
		return *this;
	}
	SEVoxelSDF::Builder& SEVoxelSDF::Builder::setTriangles(std::vector<std::array<uint32_t, 3>> triangles) {
		this->triangles = triangles;
		return *this;
	}
	SEVoxelSDF::Builder& SEVoxelSDF::Builder::setResolution(glm::ivec3 dim) {
		this->resolution = dim;
		return *this;
	}
	void SEVoxelSDF::Builder::build() {
		size_t points = resolution.x * resolution.y * resolution.z;
		distanceFieldPoints.resize(points);

		glm::vec3 boundsMin = glm::vec3(INFINITY);
		glm::vec3 boundsMax = glm::vec3(-INFINITY);
		for (glm::vec3 vert : vertices) {
			boundsMin = glm::min(vert, boundsMin);
			boundsMax = glm::max(vert, boundsMax);
		}
		// overcompensate to have a valid SDF distance on the borders
		boundsMin -= 1.0F / (glm::vec3)resolution;
		boundsMin += 1.0F / (glm::vec3)resolution;

		boundsHalfExtent = (boundsMax - boundsMin) / 2.0f;
		boundsCenter = (boundsMax + boundsMin) / 2.0f;

		int index = 0;
		for (int z = 0; z < resolution.z; ++z) {
			for (int y = 0; y < resolution.y; ++y) {
				for (int x = 0; x < resolution.x; ++x) {
					glm::vec3 uvSpace = (glm::vec3(x, y, z) + 0.5f) / static_cast<glm::vec3>(resolution);
					glm::vec3 pointInSpace = (uvSpace * 2.0f - 1.0f) * boundsHalfExtent + boundsCenter;

					findClosestTriangle(pointInSpace, distanceFieldPoints[index]);

					++index;
				}
			}
		}
	}
	int SEVoxelSDF::Builder::findClosestTriangle(glm::vec3 point, float& outSignedDist) {
		float positiveDist = INFINITY;
		float negativeDist = -INFINITY;
		int positiveTri = -1;
		int negativeTri = -1;
		for (int i = 0; i < triangles.size(); ++i) {
			float dist = distanceToTriangle(point, triangles[i]);
			if (dist < positiveDist && dist >= 0) {
				positiveDist = dist;
				positiveTri = i;
			}
			if (dist > negativeDist && dist <= 0) {
				negativeDist = dist;
				negativeTri = i;
			}
		}
		if (-negativeDist > positiveDist) {
			outSignedDist = negativeDist;
			return negativeTri;
		} else {
			outSignedDist = positiveDist;
			return positiveTri;
		}
	}
	float SEVoxelSDF::Builder::distanceToTriangle(glm::vec3 point, std::array<uint32_t, 3> triangle) {
		return glm::dot(glm::vec4(point, 1.0f), planeFromTriangle(triangle));
	}
	glm::vec4 SEVoxelSDF::Builder::planeFromTriangle(std::array<uint32_t, 3> triangle) {
		glm::vec3 ab = vertices[triangle[1]] - vertices[triangle[0]];
		glm::vec3 ac = vertices[triangle[2]] - vertices[triangle[0]];
		glm::vec3 n = glm::normalize(glm::cross(ab, ac));
		if (glm::any(glm::isnan(n)) || glm::any(glm::isinf(n))) return {};
		float d = -glm::dot(n, vertices[triangle[0]]);
		return { n, d };
	}
	SEVoxelSDF::SEVoxelSDF(SEDevice& device, SEDescriptorPool& descriptorPool, const SEVoxelSDF::Builder& builder) 
		: seDevice(device), seDescriptorPool(descriptorPool), halfExtent(builder.boundsHalfExtent) {
		createVoxel(builder);
		createDescriptorSetLayout();
		createDescriptor();
	}
	SEVoxelSDF::~SEVoxelSDF() {
		vkDeviceWaitIdle(seDevice.getDevice());
		seDescriptorPool.freeDescriptors({ distanceFieldDescriptor });
		delete distanceFieldTexture;
	}
	void SEVoxelSDF::createVoxel(const SEVoxelSDF::Builder& builder) {
		SEEmptyTextureCreateInfo createInfo{};
		createInfo.dimensions = builder.resolution;
		createInfo.format = VK_FORMAT_R16_SFLOAT;
		createInfo.imageType = VK_IMAGE_TYPE_3D;
		createInfo.layers = 1;
		createInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		createInfo.linearFiltering = true;
		createInfo.mipLevels = 1;
		createInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
		distanceFieldTexture = new SEEmptyTexture(seDevice, createInfo);

		SEBuffer stagingBuffer = SEBuffer(
			seDevice,
			2,
			builder.distanceFieldPoints.size(),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		);
		stagingBuffer.map();
		stagingBuffer.writeToBuffer(builder.distanceFieldPoints.data());
		stagingBuffer.flush();
		VkCommandBuffer cmb = seDevice.beginSingleTimeCommands();
		seDevice.transitionImageLayout(cmb, distanceFieldTexture->getImage(), VK_FORMAT_R16_SFLOAT,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1);
		seDevice.copyBufferToImage(cmb, stagingBuffer.getBuffer(), distanceFieldTexture->getImage(),
			{ (uint32_t)builder.resolution.x, (uint32_t)builder.resolution.y, (uint32_t)builder.resolution.z }, 1);
		seDevice.transitionImageLayout(cmb, distanceFieldTexture->getImage(), VK_FORMAT_R16_SFLOAT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1);
		seDevice.endSingleTimeCommands(cmb);
	}
	void SEVoxelSDF::createDescriptorSetLayout() {
		descriptorSetLayout = SEDescriptorSetLayout::Builder(seDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();
	}
	void SEVoxelSDF::createDescriptor() {
		SEDescriptorWriter(*descriptorSetLayout, seDescriptorPool)
			.writeImage(0, &distanceFieldTexture->getDescriptor())
			.build(distanceFieldDescriptor);
	}
}