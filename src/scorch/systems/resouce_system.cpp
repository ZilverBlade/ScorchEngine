#include "resource_system.h"

namespace ScorchEngine {
	ResourceSystem::ResourceSystem(SEDevice& device) : seDevice(device) {
	}
	ResourceSystem::~ResourceSystem() {
		for (auto& [id, model] : modelAssets) {
			delete model;
		}
		modelAssets.clear();
	}
	ResourceID ResourceSystem::loadModel(std::string path) {
		ResourceID id = ResourceID(path);
		auto bdr = SEModel::Builder();
		bdr.loadModel(path);
		modelAssets[id] = new SEModel(seDevice, bdr);
		return id;
	}
	SEModel* ResourceSystem::getModel(ResourceID id) {
		return modelAssets[id];
	}
	TextureResourceIDAttributes ResourceSystem::loadTexture2D(std::string path, bool srgb, bool linearSampler) {
		ResourceID id = ResourceID(path);
		auto bdr = SETexture::Builder();
		bdr.loadSTB2DImage(path);
		bdr.srgb = srgb;
		
		TextureResourceIDAttributes attribid{};
		attribid.id = id;
		attribid.srgb = srgb;
		attribid.linearSampler = linearSampler;

		texture2DAssets[attribid] = new SETexture2D(seDevice, bdr);
		return attribid;
	}
	SETexture2D* ResourceSystem::getTexture2D(TextureResourceIDAttributes id) {
		if (texture2DAssets.find(id) == texture2DAssets.end()) {
			loadTexture2D(id.id.getAsset(), id.srgb, id.linearSampler);
		}
		return texture2DAssets[id];
	}
}