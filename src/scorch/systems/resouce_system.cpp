#include "resource_system.h"

namespace ScorchEngine {
	ResourceID ResourceSystem::loadModel(SEDevice& device, std::string path) {
		ResourceID id = ResourceID(path);
		auto bdr = SEModel::Builder();
		bdr.loadModel(path);
		modelAssets[id] = new SEModel(device, bdr);
		return id;
	}
	SEModel* ResourceSystem::getModel(ResourceID id) {
		return modelAssets[id];
	}
}