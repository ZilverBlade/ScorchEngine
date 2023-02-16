#pragma once

#include <vector>
#include <scorch/log.h>
#include <scorch/vkapi/queue.h>

namespace ScorchEngine {
	struct QueueFamilyIndices {
		uint32_t presentFamily = 0;
		uint32_t presentQueueCount = 0;

		uint32_t graphicsFamily = -1;
		uint32_t graphicsQueueCount = 0;

		uint32_t computeFamily = -1;
		uint32_t computeQueueCount = 0;
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

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

		VkInstance getInstance() {
			return instance;
		}
		VkDevice getDevice() {
			return device;
		}
		VkPhysicalDevice getPhysicalDevice() {
			return physicalDevice;
		}

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		SwapChainSupportDetails getSwapChainSupport(VkSurfaceKHR surface);

		VkCommandPool getCommandPool() {
			return commandPool;
		}

		SEQueue* getAvailableQueue(SEQueueType type);
		void freeAvailableQueue(SEQueue* queue);

		const VkPhysicalDeviceProperties& getDeviceProperties() { return deviceProperties; }
		const VkPhysicalDeviceFeatures& getDeviceFeatures() { return deviceFeatures; }
		QueueFamilyIndices findPhysicalQueueFamilies() {
			return findQueueFamilies(physicalDevice);
		}

		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void copyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0) {
			VkCommandBuffer commandBuffer = beginSingleTimeCommands();
			copyBuffer(srcBuffer, dstBuffer, size, srcOffset, dstOffset);
			endSingleTimeCommands(commandBuffer);
		}

	private:

		VkPhysicalDeviceProperties deviceProperties{};
		VkPhysicalDeviceFeatures deviceFeatures{};

		bool checkValidationLayerSupport();

		void createInstance();
		void pickPhyisicalDevice();
		void createLogicalDevice();
		void createCommandPool();

		VkPhysicalDeviceFeatures requestFeatures();
		bool checkDeviceFeatureSupport(VkPhysicalDevice device);
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);

		bool isDeviceSuitable(VkPhysicalDevice device);

		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		std::vector<const char*> getRequiredExtensions();

		std::vector<SEQueue> createQueues(SEQueueType queueType, uint32_t queueCount, uint32_t queueOffset);

		void setupDebugMessenger();

		VkInstance instance{};
		VkDevice device{};
		VkPhysicalDevice physicalDevice{};

		std::vector<SEQueue> graphicsQueues{};
		std::vector<SEQueue> computeQueues{};

		VkCommandPool commandPool;

		VkDebugUtilsMessengerEXT debugMessenger{};

		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
	};
}