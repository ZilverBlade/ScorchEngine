#pragma once

#include <scorch/utils/resid.h>

namespace ScorchEngine {
	inline namespace Components {
		struct MeshComponent {
			MeshComponent() = default;
			MeshComponent(const MeshComponent&) = default;
			ResourceID mesh{};
		};
	}
}