#pragma once

#include <scorch/vkapi/device.h>

namespace ScorchEngine {
    class SETexture {
    public:
        struct Builder {
            void loadSTB2DImage(const std::string& path);
            void loadSTBCubeFolder(const std::string& path);
            void free();
            int width;
            int height;
            int depth;
            size_t dataSize;
            int layers;
            bool srgb;
            bool linearSampler;
            std::vector<void*> pixels;
        };
        SETexture(SEDevice& device);
        virtual ~SETexture();

        VkSampler getSampler() const {
            return sampler;
        }
        VkImage getImage() const {
            return image;
        }
        VkImageView getImageView() const {
            return imageView;
        }
       const VkDescriptorImageInfo& getImageInfo() const {
            return descriptor;
        }
        VkImageLayout getImageLayout() const {
            return layout;
        }
        VkExtent3D getExtent() const {
            return extent;
        }
        VkFormat getFormat() const {
            return format;
        }

        void updateDescriptor();
    protected:
        virtual void createTextureImage(const SETexture::Builder& builder) = 0;

        void createTextureImageView(VkImageViewType viewType);
        void createTextureSampler();

        VkDescriptorImageInfo descriptor{};

        SEDevice& seDevice;
        VkImage image{};
        VkDeviceMemory imageMemory{};
        VkImageView imageView{};
        VkSampler sampler{};
        VkFormat format{};
        VkFilter filter{};
        VkSamplerAddressMode addressMode{};
        VkImageLayout layout{};
        uint32_t mipLevels{ 1 };
        uint32_t layerCount{ 1 };
        float anisotropy{};
        VkExtent3D extent{};
    };

}