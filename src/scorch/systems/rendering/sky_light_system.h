#pragma once
#include <scorch/vk.h>
#include <scorch/rendering/frame_info.h>
#include <scorch/rendering/scene_ssbo.h>
#include <scorch/systems/post_fx/post_processing_fx.h>
#include <scorch/graphics/environment_map.h>

namespace ScorchEngine {
	class SkyLightSystem {
	public:
		SkyLightSystem(SEDevice& device, std::unique_ptr<SEDescriptorSetLayout>& skyLightDescriptorLayout, uint32_t framesInFlight);
		~SkyLightSystem();

		SkyLightSystem(const SkyLightSystem&) = delete;
		SkyLightSystem& operator=(const SkyLightSystem&) = delete;

		void update(FrameInfo& frameInfo, SceneSSBO& sceneBuffer, SETextureCube* skyLight);
		VkDescriptorSet getDescriptorSet(uint32_t frameIndex) {
			return descriptorSet[frameIndex];
		}
	private:
		std::unordered_map<SETextureCube*, SEEnvironmentMap*> envCubeToMap{};

		SEDevice& seDevice;
		std::unique_ptr<SEDescriptorSetLayout>& skyLightDescriptorLayout;
		std::vector<VkDescriptorSet> descriptorSet{};
	};
}