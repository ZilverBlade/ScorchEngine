#pragma once
#include <scorch/rendering/frame_info.h>
#include <scorch/rendering/scene_ssbo.h>

namespace ScorchEngine {
	class LightSystem {
	public:
		void update(FrameInfo& frameInfo, SceneSSBO& sceneBuffer);
	};
}