#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <scorch/log.h>

namespace ScorchEngine {
	class SEDevice {
	public:
		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
	};
#ifndef NDEBUG
		const bool enableValidationLayers = true;
#else
		const bool enableValidationLayers = false;
#endif
		SEDevice(const SEDevice&) = delete;
		SEDevice& operator=(const SEDevice&) = delete;
		SEDevice(SEDevice&&) = delete;
		SEDevice& operator=(SEDevice&&) = delete;

		SEDevice();
		~SEDevice();

		VkInstance getInstance();
		VkDevice getDevice();
		VkPhysicalDevice getPhysicalDevice();

	private:
		bool checkValidationLayerSupport();

		void createInstance();
		void createPhysicalDevice();
		std::vector<const char*> getRequiredExtensions();
		void createLogicalDevice();

		void setupDebugMessenger();

		VkInstance instance{};
		VkDevice device{};
		VkPhysicalDevice physicalDevice{};

		VkDebugUtilsMessengerEXT debugMessenger{};
	};
}