#pragma once

#include <scorch/window.h>
#include <scorch/vkapi/device.h>
#include <scorch/vkapi/render_pass.h>

namespace ScorchEngine {
	class SESwapChain {
	public:
        SESwapChain(SEDevice& device, SEWindow& window, VkExtent2D windowExtent, SESwapChain* oldSwapChain = nullptr);
        ~SESwapChain();

        SESwapChain(const SESwapChain&) = delete;
        SESwapChain& operator=(const SESwapChain&) = delete;
        SESwapChain() = default;

        VkFramebuffer getFrameBuffer(int index) {
            return swapChainFrameBuffers[index]->getFrameBuffer();
        }
        VkRenderPass getRenderPass() {
            return swapChainRenderPass->getRenderPass();
        }
        VkImageView getImageView(int index) {
            return swapChainAttachments[index]->getImageView();
        }
        size_t getImageCount() {
            return swapChainAttachments.size();
        }
        VkFormat getSwapChainImageFormat() {
            return swapChainImageFormat;
        }
        VkExtent2D getSwapChainExtent() {
            return swapChainExtent;
        }
        uint32_t width() {
            return swapChainExtent.width;
        }
        uint32_t height() {
            return swapChainExtent.height;
        }
        uint32_t getImageIndex() {
            return currentFrame;
        }

        float extentAspectRatio() {
            return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }

        VkResult acquireNextImage(uint32_t* imageIndex);
        VkSubmitInfo getSubmitInfo(uint32_t* imageIndex);
        VkResult present(VkQueue queue, uint32_t* imageIndex);

        VkFence getFence(uint32_t imageIndex) {
            return inFlightFences[imageIndex];
        }

        bool compareSwapFormats(const SESwapChain& swapChain) const {
            return swapChain.swapChainImageFormat == swapChainImageFormat;
        }

        void beginRenderPass(VkCommandBuffer commandBuffer) {
            swapChainRenderPass->beginRenderPass(commandBuffer, swapChainFrameBuffers[currentFrame]);
        }
        void endRenderPass(VkCommandBuffer commandBuffer) {
            swapChainRenderPass->endRenderPass(commandBuffer);
        }
    private:
        void init();
        void createSwapChain(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D extent, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode);
        void createAttachments();
        void createRenderPass();
        void createFrameBuffers();
        void createSyncObjects();
        void setImageCount(const VkSurfaceCapabilitiesKHR& capabilities);

        // Helper functions
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>&availableFormats);


        VkPresentModeKHR chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>&availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities);

        VkFormat swapChainImageFormat{};
        VkExtent2D swapChainExtent{};

        std::vector<SEFrameBuffer*> swapChainFrameBuffers{};
        std::vector<SEFrameBufferAttachment*> swapChainAttachments{};
        SERenderPass* swapChainRenderPass{};

        SEDevice& seDevice;
        SEWindow& seWindow;
        VkExtent2D windowExtent{};

        VkSwapchainKHR swapChain{};
        SESwapChain* oldSwapChain = nullptr;

        std::vector<VkSemaphore> imageAvailableSemaphores{};
        std::vector<VkSemaphore> renderFinishedSemaphores{};
        std::vector<VkFence> inFlightFences{};
        std::vector<VkFence> imagesInFlight{};
        size_t currentFrame = 0;
        uint32_t imageCount = 3;
	};
}