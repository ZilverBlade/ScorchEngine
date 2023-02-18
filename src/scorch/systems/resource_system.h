#pragma once

#include <unordered_map>
#include <scorch/utils/resid.h>
#include <scorch/graphics/model.h>

namespace ScorchEngine {
	class ResourceSystem {
	public:
		ResourceID loadModel(SEDevice& device, std::string path);
		SEModel* getModel(ResourceID id);
	private:
		std::unordered_map<ResourceID, SEModel*> modelAssets;
	};
}