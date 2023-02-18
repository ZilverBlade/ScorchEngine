#pragma once

#include <entt/entt.hpp>
#include <scorch/utils/uuid.h>

namespace ScorchEngine {
	class Actor;
	class Level {
	public:
		Level();

		Level(const Level&) = delete;
		Level& operator=(const Level&) = delete;

		Actor createActor(const std::string& name, UUID uuid = UUID());
		Actor getActor(UUID uuid);
		//Actor getActor(std::string tag);

		entt::registry& getRegistry() { return registry; }
	private:
		std::unordered_map<UUID, entt::entity> actorUUIDMap{};

		entt::registry registry;
		friend class Actor;
	};
}