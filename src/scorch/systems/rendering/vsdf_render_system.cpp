#include "vsdf_render_system.h"
#include <scorch/vkapi/pipeline_layout.h>
#include <scorch/vkapi/graphics_pipeline.h>
#include <scorch/rendering/frame_info.h>
#include <scorch/systems/resource_system.h>
#include <scorch/ecs/actor.h>
#include <scorch/graphics/sdf.h>

namespace ScorchEngine {
	struct Push {
		glm::mat4 inverseTransform;
		glm::vec3 halfExtent;
	};
	VoxelSDFRenderSystem::VoxelSDFRenderSystem(SEDevice& device, VkRenderPass renderPass, 
		VkDescriptorSetLayout uboLayout, VkSampleCountFlagBits msaaSamples)	
	: seDevice(device) {
		createPipelineLayout(uboLayout);
		createGraphicsPipeline(renderPass, msaaSamples);
	}
	VoxelSDFRenderSystem::~VoxelSDFRenderSystem() {
		delete pipeline;
		delete pipelineLayout;
	}
	void VoxelSDFRenderSystem::renderSDFs(const FrameInfo& frameInfo) {
		pipeline->bind(frameInfo.commandBuffer);
		for (entt::entity ent : frameInfo.level->getRegistry().view<TransformComponent, MeshComponent>()) {
			Actor actor = { ent, frameInfo.level.get() };
			SEModel* model = frameInfo.resourceSystem->getModel(actor.getComponent<Components::MeshComponent>().mesh);
			VkDescriptorSet descriptor = model->getSDF().getDescriptorSet();

			Push pushData;
			pushData.inverseTransform = glm::inverse(actor.getTransform().getTransformMatrix());
			pushData.halfExtent = 2.0f * model->getSDF().getHalfExtent();
			push.push(frameInfo.commandBuffer, pipelineLayout->getPipelineLayout(), &pushData);

			VkDescriptorSet descriptors[2]{
				frameInfo.globalUBO,
				descriptor
			};
			vkCmdBindDescriptorSets(
				frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout->getPipelineLayout(),
				0,
				2,
				descriptors,
				0,
				nullptr
			);
			vkCmdDraw(frameInfo.commandBuffer, 36, 1, 0, 0);
		}
	}
	void VoxelSDFRenderSystem::createPipelineLayout(VkDescriptorSetLayout uboLayout) {
		push = SEPushConstant(sizeof(Push), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		descriptorSetLayout = SEDescriptorSetLayout::Builder(seDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();
		pipelineLayout = new SEPipelineLayout(
			seDevice,
			{ push.getRange() },
			{ uboLayout, descriptorSetLayout->getDescriptorSetLayout() }
		);
	}
	void VoxelSDFRenderSystem::createGraphicsPipeline(VkRenderPass renderPass, VkSampleCountFlagBits msaaSamples) {
		SEGraphicsPipelineConfigInfo configInfo = SEGraphicsPipelineConfigInfo(1);
		configInfo.setSampleCount(msaaSamples);
		configInfo.disableDepthTest();
		configInfo.disableDepthWrite();
		configInfo.renderPass = renderPass;
		configInfo.pipelineLayout = pipelineLayout->getPipelineLayout();

		pipeline = new SEGraphicsPipeline(seDevice, configInfo, {
			SEShader(SEShaderType::Vertex, "res/shaders/spirv/vsdf_preview.vsh.spv"),
			SEShader(SEShaderType::Fragment, "res/shaders/spirv/vsdf_preview.fsh.spv")
		});
	}
}