#pragma once
#include <scorch/vkapi/device.h>
namespace ScorchEngine {
	enum class SEShaderType {
		Vertex,
		Geometry,
		Fragment,
		Compute
	};
	class SEShader {
	public:
		SEShader(
			SEShaderType type,
			const char* shaderLocation
		);
		
		~SEShader();

		VkShaderModule createShaderModule(SEDevice& device) const;
		VkShaderStageFlagBits getVkShaderStage() const { 
			switch (type) {
			case ScorchEngine::SEShaderType::Vertex:
				return VK_SHADER_STAGE_VERTEX_BIT;
			case ScorchEngine::SEShaderType::Geometry:
				return VK_SHADER_STAGE_GEOMETRY_BIT;
			case ScorchEngine::SEShaderType::Fragment:
				return VK_SHADER_STAGE_FRAGMENT_BIT;

			case ScorchEngine::SEShaderType::Compute:
				return VK_SHADER_STAGE_COMPUTE_BIT;
			}
		}
	private:
		SEShaderType type;
		const char* file;
		std::vector<char> buffer;
	};
}