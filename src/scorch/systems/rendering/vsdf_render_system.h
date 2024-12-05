#pragma once
#include <scorch/vkapi/push_constant.h>
#include <scorch/vkapi/device.h>
#include <scorch/vkapi/descriptors.h>
#include <glm/vec2.hpp>
namespace ScorchEngine {
	class SEGraphicsPipeline;
	class SEPipelineLayout;
	class SEFramebuffer;
	class SEFramebufferAttachment;
	class SERenderPass;
	class SEDescriptorPool;
	class Actor;
	struct FrameInfo;

	const float VSDF_SHADOW_QUALITY = 0.5f;

	class VoxelSdfRenderSystem {
	public:
		VoxelSdfRenderSystem(SEDevice& device, glm::vec2 size,
			VkDescriptorImageInfo depthTarget, VkDescriptorSetLayout uboLayout);
		~VoxelSdfRenderSystem();

		VoxelSdfRenderSystem(const VoxelSdfRenderSystem&) = delete;
		VoxelSdfRenderSystem& operator=(const VoxelSdfRenderSystem&) = delete;
		VoxelSdfRenderSystem(VoxelSdfRenderSystem&&) = delete;
		VoxelSdfRenderSystem& operator=(VoxelSdfRenderSystem&&) = delete;

		void renderSdfShadows(const FrameInfo& frameInfo, Actor sun);

		VkDescriptorImageInfo getShadowMaskDescriptor();

		void resize(glm::vec2 size, VkDescriptorImageInfo depthTarget);
	protected:
		void createRenderTargets(glm::vec2 size);
		void createRenderPasses();
		void createFramebuffers();

		void createPipelineLayout(VkDescriptorSetLayout uboLayout);
		void createGraphicsPipeline();

		void rebuildDepthDescriptor(VkDescriptorImageInfo depthTarget);

		SEDevice& seDevice;

		SERenderPass* shadowMaskRenderPass;
		SEFramebuffer* shadowMaskFramebuffer;
		SEFramebufferAttachment* shadowMaskRenderTarget;

		VkDescriptorSet depthDescriptor = VK_NULL_HANDLE;

		SEPushConstant push{};
		std::unique_ptr<SEDescriptorSetLayout> sdfDescriptorSetLayout;
		std::unique_ptr<SEDescriptorSetLayout> depthDescriptorSetLayout;
		SEPipelineLayout* pipelineLayout{};
		SEGraphicsPipeline* pipeline{};
	};
}