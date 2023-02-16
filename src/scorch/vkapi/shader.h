#pragma once
#include <scorch/vkapi/device.h>
namespace ScorchEngine {
	enum class SEShaderType {
		Vertex,
		Fragment
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
				
			case ScorchEngine::SEShaderType::Fragment:
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			}
		}
	private:
		SEShaderType type;
		const char* file;
		std::vector<char> buffer;
	};
}