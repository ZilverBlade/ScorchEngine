#pragma once
#include <string>

namespace ScorchEngine {
	using ResourceKey = uint64_t;

	class ResourceID {
	public:
		ResourceID(const std::string& asset);
		ResourceID() : file("\0"), id(0) {}
		uint64_t getID() const { return id; }
		std::string getAsset() const { return file; }
		operator ResourceKey() const { return getID(); }
	private:
		std::string file;
		uint64_t id;
	};

}
namespace std {
	template<>
	struct hash<ScorchEngine::ResourceID> {
		size_t operator()(const ScorchEngine::ResourceID& id) const {
			return id.getID();
		}
	};
}