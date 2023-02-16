#include "shader.h"
#include <fstream>

namespace ScorchEngine {
	
	SEShader::SEShader(SEShaderType type, const char* shaderLocation) : file(shaderLocation), type(type) {
		std::ifstream stream{shaderLocation, std::ios::ate | std::ios::binary };
		size_t fileSize = static_cast<size_t>(stream.tellg());

		if (!stream.is_open()) {
			throw std::runtime_error("failed to open file: " + std::string(shaderLocation));
		}

		buffer.resize(static_cast<size_t>(stream.tellg()));
		stream.seekg(0);
		stream.read(buffer.data(), fileSize);
		
		stream.close();
	}

	SEShader::~SEShader() {
	}

	VkShaderModule SEShader::createShaderModule(SEDevice& device) const {
		VkShaderModule shaderModule{};
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = buffer.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

		if (vkCreateShaderModule(device.getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
		return shaderModule;
	}
}