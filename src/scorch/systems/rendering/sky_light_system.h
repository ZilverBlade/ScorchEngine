#pragma once
#include <scorch/vk.h>
#include <scorch/rendering/frame_info.h>
#include <scorch/rendering/scene_ssbo.h>
#include <scorch/systems/post_fx/post_processing_fx.h>
#include <scorch/graphics/sky_light.h>

namespace ScorchEngine {
	class SkyLightSystem {
	public:
		SkyLightSystem(SEDevice& device, SEDescriptorPool& descriptorPool, uint32_t framesInFlight);
		~SkyLightSystem();

		SkyLightSystem(const SkyLightSystem&) = delete;
		SkyLightSystem& operator=(const SkyLightSystem&) = delete;

		void update(SESkyLight* skyLight);

	private:
		SEDevice& seDevice;
		SEDescriptorPool& seDescriptorPool;
		std::unique_ptr<SEDescriptorSetLayout> skyLightDescriptorLayout;

		SEPipelineLayout* pipelineLayout{};
		SEGraphicsPipeline* pipeline{};

		SETextureCube* environment{};

		SEPostProcessingEffect* envBRDFGen{};
		SEPostProcessingEffect* envPrefilteredGen{};
	};
}