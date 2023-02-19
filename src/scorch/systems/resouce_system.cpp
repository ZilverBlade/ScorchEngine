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
}