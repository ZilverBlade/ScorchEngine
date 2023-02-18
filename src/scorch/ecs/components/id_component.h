#pragma once

#include <string>
#include <scorch/utils/uuid.h>

namespace ScorchEngine {
	inline namespace Components {
		struct IDComponent {
			IDComponent(const IDComponent&) = default;
			IDComponent(UUID uuid, std::string tag) : uuid(uuid), tag(tag) {}
			UUID getUUID() { 
				return uuid;
			}
			std::string tag;
		private:
			UUID uuid;
			friend class Actor;
			friend class Level;
		};
	}
}