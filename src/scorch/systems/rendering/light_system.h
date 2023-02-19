#pragma once
#include <scorch/renderer/frame_info.h>
#include <scorch/renderer/scene_ssbo.h>

namespace ScorchEngine {
	class LightSystem {
	public:
		void update(FrameInfo& frameInfo, SceneSSBO& sceneBuffer);
	};
}