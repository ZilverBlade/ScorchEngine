#pragma once

#include <unordered_map>
#include <scorch/utils/resid.h>
#include <scorch/graphics/model.h>

namespace ScorchEngine {
	class ResourceSystem {
	public:
		ResourceSystem(SEDevice& device);
		~ResourceSystem();
		ResourceID loadModel(std::string path);
		SEModel* getModel(ResourceID id);
	private:
		SEDevice& seDevice;
		std::unordered_map<ResourceID, SEModel*> modelAssets;
	};
}