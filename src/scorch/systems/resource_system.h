#pragma once

#include <unordered_map>
#include <scorch/utils/resid.h>
#include <scorch/graphics/model.h>
#include <scorch/graphics/texture2d.h>
#include <scorch/graphics/texture_cube.h>
#include <scorch/graphics/surface_material.h>

namespace ScorchEngine {
	struct TextureResourceIDAttributes {
		ResourceID id{};
		bool srgb{};
		bool linearSampler{};
		bool operator==(ScorchEngine::TextureResourceIDAttributes const& other) const {
			return id.getID() == other.id.getID() && srgb == other.srgb && linearSampler == other.linearSampler;
		}
	};
}
namespace std {
	template<>
	struct hash<ScorchEngine::TextureResourceIDAttributes> {
		size_t operator()(ScorchEngine::TextureResourceIDAttributes const& attribid) const {
			return attribid.id.getID() << 2 || attribid.srgb << 1 || attribid.linearSampler;
		}
	};
}

namespace ScorchEngine {
	class ResourceSystem {
	public:
		ResourceSystem(SEDevice& device, SEDescriptorPool& descriptorPool);
		~ResourceSystem();

		ResourceSystem(const ResourceSystem&) = delete;
		ResourceSystem& operator=(const ResourceSystem&) = delete;

		ResourceID loadModel(std::string path);
		SEModel* getModel(ResourceID id);

		TextureResourceIDAttributes loadTexture2D(std::string path, bool srgb, bool linearSampler);
		SETexture2D* getTexture2D(TextureResourceIDAttributes id);

		TextureResourceIDAttributes loadTextureCube(std::string path, bool srgb = true, bool linearSampler = true);
		SETextureCube* getTextureCube(TextureResourceIDAttributes id);

		ResourceID loadSurfaceMaterial(std::string path);
		SESurfaceMaterial* getSurfaceMaterial(ResourceID id);

		SETexture2D* getMissingTexture2D() {
			return special_MissingTexture2D;
		}
	private:
		SEDevice& seDevice;
		SEDescriptorPool& seDescriptorPool;

		std::unordered_map<ResourceID, SEModel*> modelAssets;
		std::unordered_map<TextureResourceIDAttributes, SETexture2D*> texture2DAssets;
		std::unordered_map<TextureResourceIDAttributes, SETextureCube*> textureCubeAssets;
		std::unordered_map<ResourceID, SESurfaceMaterial*> surfaceMaterialAssets;

		SETexture2D* special_MissingTexture2D;
	};
}