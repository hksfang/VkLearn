#include <AllocatedImage.h>

#include "AllocatedBuffer.h"
#include "ImmediateContext.h"
#include "VkCmds.h"
#include "VkContext.h"
#include "VkDebug.h"
#include "VkInit.h"
#include "Core/Debug.h"

AllocatedImage::AllocatedImage(
    VkImageCreateFlags createFlags,
    uint32_t arrayLayers,
    VkImageUsageFlags imageUsageFlags,
    const VkExtent3D &extent,
    VkFormat format,
    bool mipmapped) : extent3D_(extent),
                      imageFormat_(format) {
    mipLevels_ = mipmapped
                             ? static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1
                             : 1;

    auto imageCreateInfo = InitImageCreateInfo(
        format,
        createFlags,
        arrayLayers,
        imageUsageFlags,
        extent,
        mipLevels_);
    auto allocationInfo = InitVmaAllocationCreateInfo(VMA_MEMORY_USAGE_GPU_ONLY,
                                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
    VK_CHECK(vmaCreateImage(GetAllocator(), &imageCreateInfo, &allocationInfo, &image_, &vmaAllocation_, nullptr));
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    // TODO: We're just assuming here to be simple.
    if (format == VK_FORMAT_D32_SFLOAT) {
        aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    // TODO: Remove this hack!
    auto viewType = VK_IMAGE_VIEW_TYPE_2D;
    if (arrayLayers != 1) {
        viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    }
    if (arrayLayers == 6) {
        viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    }
    auto imageViewCreateInfo =
            InitImageViewCreateInfo(
                format,
                image_,
                aspectFlags,
                viewType,
                arrayLayers,
                mipLevels_
            );
    VK_CHECK(vkCreateImageView(GetDevice(), &imageViewCreateInfo, nullptr, &imageView_));
}

AllocatedImage::AllocatedImage(
    void *data,
    VkExtent3D size,
    size_t pixelSize,
    VkFormat format,
    VkImageCreateFlags createFlags,
    uint32_t arrayLayers,
    VkImageUsageFlags imageUsageFlags,
    bool mipmapped)
    : AllocatedImage(
        createFlags,
        arrayLayers,
        imageUsageFlags,
        size,
        format,
        mipmapped
    ) {
    size_t dataSize = arrayLayers * size.depth * size.width * size.height * pixelSize;
    auto uploadBuffer = AllocatedBuffer::New(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    CHECK(uploadBuffer->GetMappedData<void>());
    memcpy(uploadBuffer->GetMappedData<void>(), data, dataSize);

    ImmediateContext::GetInstance().ImmediateSubmit([&](VkCommandBuffer cmd) {
        ImageTransistion(cmd, image_, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        VkBufferImageCopy copyRegion{
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource{
                // TODO: Support other aspects?
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = arrayLayers,
            },
            .imageExtent = size
        };
        vkCmdCopyBufferToImage(cmd, uploadBuffer->GetBuffer(), image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                               &copyRegion);

        if (mipLevels_ > 1) {
            for (uint32_t i = 1; i < mipLevels_; i++) {
                VkImageBlit blit{};
                blit.srcOffsets[0] = {0, 0, 0};
                blit.srcOffsets[1] = {
                    static_cast<int32_t>(size.width >> (i - 1)),
                    static_cast<int32_t>(size.height >> (i - 1)),
                    1
                };
                blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount = arrayLayers;
                blit.dstOffsets[0] = {0, 0, 0};
                blit.dstOffsets[1] = {
                    static_cast<int32_t>(std::max(1u, size.width >> i)),
                    static_cast<int32_t>(std::max(1u, size.height >> i)),
                    1
                };
                blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount = arrayLayers;

                ImageTransistion(cmd, image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 i - 1, 1);
                vkCmdBlitImage(cmd, image_, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image_,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);
                ImageTransistion(cmd, image_, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, i - 1, 1);
            }
            ImageTransistion(cmd, image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                             mipLevels_ - 1, 1);
        } else {
            ImageTransistion(cmd, image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    });
}


AllocatedImage::~AllocatedImage() {
    vkDestroyImageView(GetDevice(), imageView_, nullptr);
    vmaDestroyImage(GetAllocator(), image_, vmaAllocation_);
}
