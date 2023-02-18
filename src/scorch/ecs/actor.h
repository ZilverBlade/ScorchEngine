#pragma once

#include <entt/entt.hpp>
#include <scorch/ecs/level.h>
#include <scorch/ecs/components.h>
namespace ScorchEngine {
	class Actor {
	public:
		Actor() = default;
		Actor(const Actor& other) = default;
		Actor(entt::entity handle, Level* level);

		template<typename T>
		bool hasComponent() {
			return level->registry.all_of<T>(handle);
		}

		template<typename T>
		T& getComponent() {
			assert(hasComponent<T>());
			return level->registry.get<T>(handle);
		}

		template<typename T, typename... Args>
		T& addComponent(Args&&... args) {
			if (hasComponent<T>()) {
				return getComponent<T>();
			}
			return level->registry.emplace<T>(handle, std::forward<Args>(args)...);
		}

		template<typename T>
		T& rmvComponent() {
			if (hasComponent<T>()) {
				return level->registry.remove<T>(handle);
			}
		}

		UUID getUUID() {
			return getComponent<Components::IDComponent>().getUUID();
		}
		std::string& getTag() {
			return getComponent<Components::IDComponent>().tag;
		}
		TransformComponent& getTransform() {
			return getComponent<Components::TransformComponent>();
		}

		operator bool() { return handle != entt::null; }
		operator entt::entity() { return handle; };

		bool operator==(const Actor& other) {
			return handle == other.handle && level == other.level;
		}
		bool operator!=(const Actor& other) {
			return !(*this == other);
		}

		const entt::entity handle = entt::null;
	private:
		Level* level = nullptr;
	};
}