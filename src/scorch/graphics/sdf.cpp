#include "sdf.h"
#include <algorithm>
#include <thread>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

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
		boundsMax += 1.0F / (glm::vec3)resolution;

		boundsHalfExtent = (boundsMax - boundsMin) / 2.0f;
		boundsCenter = (boundsMax + boundsMin) / 2.0f;

		int maxDispatches = std::thread::hardware_concurrency();
		int zDispatches = std::ceil(resolution.z / static_cast<float>(maxDispatches));
		int zOffset = 0;
		std::vector<std::thread> dispatches;
		for (int i = 0; i < maxDispatches; ++i) {
			if (zOffset >= resolution.z) break;
			dispatches.push_back(std::thread([=]() {
				for (int z = zOffset; z < zOffset + zDispatches && z < resolution.z; ++z) {
					int index = z * resolution.y * resolution.x;
					for (int y = 0; y < resolution.y; ++y) {
						for (int x = 0; x < resolution.x; ++x) {
							glm::vec3 uvSpace = (glm::vec3(x, y, z) + 0.5f) / static_cast<glm::vec3>(resolution);
							glm::vec3 pointInSpace = (uvSpace * 2.0f - 1.0f) * boundsHalfExtent + boundsCenter;

							findClosestTriangle(pointInSpace, distanceFieldPoints[index]);
							++index;
						}
					}
				}
			}));
			zOffset += zDispatches;
		}

		for (auto& dispatch : dispatches) {
			if (dispatch.joinable()) {
				dispatch.join();
			}
		}
	}
	int SEVoxelSDF::Builder::findClosestTriangle(glm::vec3 point, float& outSignedDist) {
		float positiveSqDist = INFINITY;
		float negativeSqDist = -INFINITY;
		int positiveTri = -1;
		int negativeTri = -1;
		for (int i = 0; i < triangles.size(); ++i) {
			float sqdist = signedDistanceSquareToTriangle(point, triangles[i]);
			if (sqdist < positiveSqDist && sqdist >= 0) {
				positiveSqDist = sqdist;
				positiveTri = i;
			}
			if (sqdist > negativeSqDist && sqdist <= 0) {
				negativeSqDist = sqdist;
				negativeTri = i;
			}
		}
		if (-negativeSqDist < positiveSqDist) {
			outSignedDist = -sqrt(-negativeSqDist);
			return negativeTri;
		} else {
			outSignedDist = sqrt(positiveSqDist);
			return positiveTri;
		}

		outSignedDist = sqrt(std::min(-negativeSqDist, positiveSqDist));
		return negativeTri;
		
	}
	float SEVoxelSDF::Builder::signedDistanceSquareToTriangle(glm::vec3 p, std::array<uint32_t, 3> triangle) {
		glm::vec3 v21 = vertices[triangle[1]] - vertices[triangle[0]];
		glm::vec3 v32 = vertices[triangle[2]] - vertices[triangle[1]];
		glm::vec3 v13 = vertices[triangle[0]] - vertices[triangle[2]];
		glm::vec3 nor = cross(v21, v13);
		float d = -glm::dot(nor, vertices[triangle[0]]);

		glm::vec3 p1 = p - vertices[triangle[0]];
		glm::vec3 p2 = p - vertices[triangle[1]];
		glm::vec3 p3 = p - vertices[triangle[2]];

		bool x = sqrt( // inside/outside test    
			(glm::sign(dot(cross(v21, nor), p1)) +
				glm::sign(dot(cross(v32, nor), p2)) +
				glm::sign(dot(cross(v13, nor), p3)) < 2.0));
		float above = -glm::sign(dot(p, nor) + d);

		if (x) {
			// 3 edges    
			return above * glm::min(glm::min(
				glm::length2(v21 * glm::clamp(dot(v21, p1) / glm::length2(v21), 0.0f, 1.0f) - p1),
				glm::length2(v32 * glm::clamp(dot(v32, p2) / glm::length2(v32), 0.0f, 1.0f) - p2)),
				glm::length2(v13 * glm::clamp(dot(v13, p3) / glm::length2(v13), 0.0f, 1.0f) - p3));
		} else {
			// 1 face    
			return above * dot(nor, p1)* dot(nor, p1) / glm::length2(nor);
		}

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
		createInfo.format = VK_FORMAT_R32_SFLOAT;
		createInfo.imageType = VK_IMAGE_TYPE_3D;
		createInfo.layers = 1;
		createInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		createInfo.linearFiltering = true;
		createInfo.mipLevels = 1;
		createInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
		distanceFieldTexture = new SEEmptyTexture(seDevice, createInfo);

		std::vector<uint16_t> fp16distances = std::vector<uint16_t>(builder.distanceFieldPoints.size());
		
		// convert everything to fp16 
		for (int i = 0; i < fp16distances.size(); ++i) {
			//assuming neither NAN nor +-INF( IEEE 754-2008  standard)
			uint16_t half = 0;
			uint32_t single = reinterpret_cast<const uint32_t&>(builder.distanceFieldPoints[i]);
			int32_t exponent_e127 = ((single & 0x7F8) >> 23);
			half |= (single & 0x80000000) >> 16; // sign
			int32_t mantissa23 = single & 0x7FFFFF;
			int32_t mantissa9 = mantissa23 >> 14; // truncate bits after the 9th MSB
			half |= mantissa9;
			if (exponent_e127 != 0) {
				int32_t exponent_e15 = std::clamp(exponent_e127 - 127, -14, 15) + 15;
				half |= exponent_e15 << 9;
			}
			fp16distances[i] = half;
		}

		SEBuffer stagingBuffer = SEBuffer(
			seDevice,
			4,
			fp16distances.size(),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		);
		stagingBuffer.map();
		stagingBuffer.writeToBuffer(builder.distanceFieldPoints.data());
		stagingBuffer.flush();
		VkCommandBuffer cmb = seDevice.beginSingleTimeCommands();
		seDevice.transitionImageLayout(cmb, distanceFieldTexture->getImage(), createInfo.format,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1);
		seDevice.copyBufferToImage(cmb, stagingBuffer.getBuffer(), distanceFieldTexture->getImage(),
			{ (uint32_t)builder.resolution.x, (uint32_t)builder.resolution.y, (uint32_t)builder.resolution.z }, 1);
		seDevice.transitionImageLayout(cmb, distanceFieldTexture->getImage(), createInfo.format,
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