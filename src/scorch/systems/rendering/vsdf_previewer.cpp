#include "vsdf_previewer.h"
#include <scorch/vkapi/pipeline_layout.h>
#include <scorch/vkapi/graphics_pipeline.h>
#include <scorch/rendering/frame_info.h>
#include <scorch/systems/resource_system.h>
#include <scorch/ecs/actor.h>
#include <scorch/graphics/sdf.h>

namespace ScorchEngine {
	struct Push {
		glm::mat4 invTransform;
		glm::vec3 halfExtent;
		int align;
		glm::vec3 scale;
	};
	VoxelSdfPreviewer::VoxelSdfPreviewer(SEDevice& device, VkRenderPass renderPass, 
		VkDescriptorSetLayout uboLayout, VkSampleCountFlagBits msaaSamples)	
	: seDevice(device) {
		createPipelineLayout(uboLayout);
		createGraphicsPipeline(renderPass, msaaSamples);
	}
	VoxelSdfPreviewer::~VoxelSdfPreviewer() {
		delete pipeline;
		delete pipelineLayout;
	}
	void VoxelSdfPreviewer::renderSdfs(const FrameInfo& frameInfo) {
		pipeline->bind(frameInfo.commandBuffer);
		for (entt::entity ent : frameInfo.level->getRegistry().view<TransformComponent, MeshComponent>()) {
			Actor actor = { ent, frameInfo.level.get() };
			SEModel* model = frameInfo.resourceSystem->getModel(actor.getComponent<Components::MeshComponent>().mesh);
			VkDescriptorSet descriptor = model->getSdf().getDescriptorSet();

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
			auto& transform = actor.getTransform();
			Push pushData;
			glm::mat4 transformMat = transform.getTransformMatrix();
			transformMat[3] +=glm::vec4( model->getSdf().getCenter(), 0.0f);
			pushData.invTransform = glm::inverse(transformMat);
			pushData.halfExtent = model->getSdf().getHalfExtent();
			pushData.scale = transform.scale;
			push.push(frameInfo.commandBuffer, pipelineLayout->getPipelineLayout(), &pushData);
			vkCmdDraw(frameInfo.commandBuffer, 36, 1, 0, 0);
		}
	}
	void VoxelSdfPreviewer::createPipelineLayout(VkDescriptorSetLayout uboLayout) {
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
	void VoxelSdfPreviewer::createGraphicsPipeline(VkRenderPass renderPass, VkSampleCountFlagBits msaaSamples) {
		SEGraphicsPipelineConfigInfo configInfo = SEGraphicsPipelineConfigInfo(1);
		configInfo.setSampleCount(msaaSamples);
		//configInfo.disableDepthTest();
		//configInfo.disableDepthWrite();
		configInfo.setCullMode(VK_CULL_MODE_FRONT_BIT);
		configInfo.renderPass = renderPass;
		configInfo.pipelineLayout = pipelineLayout->getPipelineLayout();

		pipeline = new SEGraphicsPipeline(seDevice, configInfo, {
			SEShader(SEShaderType::Vertex, "res/shaders/spirv/vsdf_preview.vsh.spv"),
			SEShader(SEShaderType::Fragment, "res/shaders/spirv/vsdf_preview.fsh.spv")
		});
	}
}