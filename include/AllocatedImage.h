#pragma once
#include <vk_mem_alloc.h>
#include "VkObjects.h"


class AllocatedImage {
public:
    explicit AllocatedImage(
        VkImageCreateFlags createFlags,
        uint32_t arrayLayers,
        VkImageUsageFlags imageUsageFlags,
        const VkExtent3D &extent,
        VkFormat format,
        bool mipmapped = false);

    // Create uploaded.
    explicit AllocatedImage(
        void *data,
        VkExtent3D size,
        size_t pixelSize,
        VkFormat format,
        VkImageCreateFlags createFlags,
        uint32_t arrayLayers,
        VkImageUsageFlags imageUsageFlags,
        bool mipmapped = false);

    ~AllocatedImage();

    const VkExtent3D &GetExtent() const {
        return extent3D_;
    }

    VkImage GetImage() const {
        return image_;
    }

    VkImageView GetImageView() const {
        return imageView_;
    }

    VkFormat GetFormat() const {
        return imageFormat_;
    }

    uint32_t GetMipLevels() const {
        return mipLevels_;
    }

private:
    VkImage image_{VK_NULL_HANDLE};
    VkImageView imageView_{VK_NULL_HANDLE};
    VmaAllocation vmaAllocation_{VK_NULL_HANDLE};
    VkExtent3D extent3D_;
    VkFormat imageFormat_;
    uint32_t mipLevels_{1};
};
