#include "resource_system.h"

namespace ScorchEngine {
	ResourceSystem::ResourceSystem(SEDevice& device, SEDescriptorPool& descriptorPool) : seDevice(device), seDescriptorPool(descriptorPool) {
		special_MissingTexture2D = getTexture2D(loadTexture2D("res/textures/missing.png", false, false));
	}
	ResourceSystem::~ResourceSystem() {
		for (auto& [id, model] : modelAssets) {
			delete model;
		}
		for (auto& [id, texture] : texture2DAssets) {
			delete texture;
		}
		for (auto& [id, texture] : textureCubeAssets) {
			delete texture;
		}
		for (auto& [id, material] : surfaceMaterialAssets) {
			delete material;
		}
	}
	ResourceID ResourceSystem::loadModel(std::string path) {
		SELOG_INF("Loading model %s", path.c_str());
		ResourceID id = ResourceID(path);
		auto bdr = SEModel::Builder();
		bdr.loadModel(path);
		modelAssets[id] = new SEModel(seDevice, seDescriptorPool, bdr);
		return id;
	}
	SEModel* ResourceSystem::getModel(ResourceID id) {
		auto iter = modelAssets.find(id);
		if (iter == modelAssets.end()) {
			throw std::runtime_error("model '" + id.getAsset() + "' not found");
		} else {
			return (*iter).second;
		}
	}
	TextureResourceIDAttributes ResourceSystem::loadTexture2D(std::string path, bool srgb, bool linearSampler) {
		SELOG_INF("Loading texture %s", path.c_str());
		ResourceID id = ResourceID(path);
		auto bdr = SETexture::Builder();
		bdr.loadSTB2DImage(path);
		bdr.srgb = srgb;
		bdr.linearSampler = linearSampler;
		
		TextureResourceIDAttributes attribid{};
		attribid.id = id;
		attribid.srgb = srgb;
		attribid.linearSampler = linearSampler;

		texture2DAssets[attribid] = new SETexture2D(seDevice, bdr);
		bdr.free();
		return attribid;
	}
	SETexture2D* ResourceSystem::getTexture2D(TextureResourceIDAttributes id) {
		auto iter = texture2DAssets.find(id);
		if (iter == texture2DAssets.end()) {
			loadTexture2D(id.id.getAsset(), id.srgb, id.linearSampler);
			return texture2DAssets[id];
		} else {
			return (*iter).second;
		}
	}
	TextureResourceIDAttributes ResourceSystem::loadTextureCube(std::string path, bool srgb, bool linearSampler) {
		SELOG_INF("Loading cubemap %s", path.c_str());
		ResourceID id = ResourceID(path);
		auto bdr = SETexture::Builder();
		bdr.loadSTBCubeFolder(path);
		bdr.srgb = srgb;

		TextureResourceIDAttributes attribid{};
		attribid.id = id;
		attribid.srgb = srgb;
		attribid.linearSampler = linearSampler;

		textureCubeAssets[attribid] = new SETextureCube(seDevice, bdr);
		bdr.free();
		return attribid;
	}
	SETextureCube* ResourceSystem::getTextureCube(TextureResourceIDAttributes id) {
		auto iter = textureCubeAssets.find(id);
		if (iter == textureCubeAssets.end()) {
			loadTextureCube(id.id.getAsset(), id.srgb, id.linearSampler);
			return textureCubeAssets[id];
		} else {
			return (*iter).second;
		}
	}
	ResourceID ResourceSystem::loadSurfaceMaterial(std::string path) {
		SELOG_INF("Loading surface material %s", path.c_str());
		ResourceID id = ResourceID(path);
		SESurfaceMaterial* material = new SESurfaceMaterial(seDevice, seDescriptorPool, this);
		material->load(path);
		material->updateParams();
		material->updateTextures();
		surfaceMaterialAssets[id] = material;
		return id;
	}
	SESurfaceMaterial* ResourceSystem::getSurfaceMaterial(ResourceID id) {
		auto iter = surfaceMaterialAssets.find(id);
		if (iter == surfaceMaterialAssets.end()) {
			throw std::runtime_error("surface material '" + id.getAsset() + "'not found");
		} else {
			return (*iter).second;
		}
	}
	bool ResourceSystem::insertSurfaceMaterial(ResourceID id, SESurfaceMaterial* sfMaterial) {
		auto iter = surfaceMaterialAssets.find(id);
		if (iter == surfaceMaterialAssets.end()) {
			surfaceMaterialAssets[id] = sfMaterial;
			return true;
		} 
		return false;
	}
}