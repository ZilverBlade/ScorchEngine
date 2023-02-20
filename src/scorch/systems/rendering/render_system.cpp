#include "render_system.h"
#include <scorch/ecs/components.h>
#include <scorch/systems/resource_system.h>
namespace ScorchEngine {
	RenderSystem::RenderSystem(
		SEDevice& device, 
		glm::vec2 size,
		VkDescriptorSetLayout uboLayout,
		VkDescriptorSetLayout ssboLayout
	) : seDevice(device) {
	//	materialSystem = std::make_unique<MaterialSystem>(device);
	}
	RenderSystem::~RenderSystem() {
	}
	void RenderSystem::init(glm::vec2 size) {
	}
	void RenderSystem::destroy()
	{
	}
	void RenderSystem::renderMeshes(FrameInfo& frameInfo, SEPushConstant push, VkPipelineLayout pipelineLayout, uint32_t descriptorOffset, bool translucent, bool doubleSided) {
		frameInfo.level->getRegistry().view<Components::TransformComponent, Components::MeshComponent>().each(
			[&](auto& tfc, auto& msc) {
				SEModel* model = frameInfo.resourceSystem->getModel(msc.mesh);
				ModelMatrixPush mpush{};
				mpush.transform = tfc.getTransformMatrix();
				mpush.normal = tfc.getNormalMatrix();
				push.push(frameInfo.commandBuffer, pipelineLayout, &mpush);

				for (const auto& [mapTo, matAsset] : msc.materials) {
					SESurfaceMaterial* material = frameInfo.resourceSystem->getSurfaceMaterial(matAsset);
					if (material->translucent != translucent || material->doubleSided != doubleSided) continue;
					material->bind(frameInfo.commandBuffer, pipelineLayout, descriptorOffset);
					model->bind(frameInfo.commandBuffer, mapTo);
					model->draw(frameInfo.commandBuffer);
				}
			}
		);
	}
}