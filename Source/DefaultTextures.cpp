#include "DefaultTextures.h"
#include "VkInit.h"
#include "VkContext.h"
#include "../include/BindlessDescriptorSet.h"

DefaultTextures::DefaultTextures() {
    uint32_t white = glm::packUnorm4x8(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    white_ = std::make_unique<AllocatedImage>(
        &white,
        VkExtent3D{1, 1, 1},
        4,
        VK_FORMAT_R8G8B8A8_UNORM, 0, 1,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    uint32_t grey = glm::packUnorm4x8(glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));
    grey_ = std::make_unique<AllocatedImage>(&grey, VkExtent3D{1, 1, 1}, 4, VK_FORMAT_R8G8B8A8_UNORM, 0, 1,
                                             VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    uint32_t black = glm::packUnorm4x8(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    black_ = std::make_unique<AllocatedImage>(&black, VkExtent3D{1, 1, 1}, 4, VK_FORMAT_R8G8B8A8_UNORM, 0, 1,
                                              VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    uint32_t flatNormal = glm::packUnorm4x8(glm::vec4(0.5f, 0.5f, 1.0f, 1.0f));
    flatNormal_ = std::make_unique<AllocatedImage>(
        &flatNormal, VkExtent3D{1, 1, 1}, 4, VK_FORMAT_R8G8B8A8_UNORM, 0, 1,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    // Checkerboard
    uint32_t magenta = glm::packUnorm4x8(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
    std::array<uint32_t, 16 * 16> pixels{};
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            pixels[y * 16 + x] = (x % 2) ^ (y % 2) ? magenta : black;
        }
    }
    errorCheckerboard_ = std::make_unique<AllocatedImage>(pixels.data(), VkExtent3D{16, 16, 1}, 4,
                                                          VK_FORMAT_R8G8B8A8_UNORM, 0, 1,
                                                          VK_IMAGE_USAGE_SAMPLED_BIT |
                                                          VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    whiteIndex_ = BindlessDescriptorSet::GetInstance().RegisterImage(white_->GetImageView());
    blackIndex_ = BindlessDescriptorSet::GetInstance().RegisterImage(black_->GetImageView());
    greyIndex_ = BindlessDescriptorSet::GetInstance().RegisterImage(grey_->GetImageView());
    flatNormalIndex_ = BindlessDescriptorSet::GetInstance().RegisterImage(flatNormal_->GetImageView());
    errorCheckerboardIndex_ = BindlessDescriptorSet::GetInstance().RegisterImage(errorCheckerboard_->GetImageView());
}

DefaultSamplers::DefaultSamplers() {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(GetPhysicalDevice(), &properties);

    auto linearSamplerInfo = InitSamplerCreateInfo(
        VK_FILTER_LINEAR,
        VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR,
        0.0f,
        VK_LOD_CLAMP_NONE,
        VK_SAMPLER_ADDRESS_MODE_REPEAT
    );
    linearSamplerInfo.anisotropyEnable = VK_TRUE;
    linearSamplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    linear_ = Sampler::New(linearSamplerInfo);

    nearest_ = Sampler::New(
        InitSamplerCreateInfo(
            VK_FILTER_NEAREST,
            VK_FILTER_NEAREST,
            VK_SAMPLER_MIPMAP_MODE_NEAREST,
            0.0f,
            VK_LOD_CLAMP_NONE,
            VK_SAMPLER_ADDRESS_MODE_REPEAT
        )
    );
    auto info =
            InitSamplerCreateInfo(VK_FILTER_LINEAR,
                                  VK_FILTER_LINEAR,
                                  VK_SAMPLER_MIPMAP_MODE_LINEAR,
                                  0.0f,
                                  VK_LOD_CLAMP_NONE,
                                  VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    info.compareEnable = VK_TRUE;
    info.compareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
    info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    compare_ = Sampler::New(
        info
    );

    linearIndex_ = BindlessDescriptorSet::GetInstance().RegisterSampler(linear_->GetVkHandle());
    nearestIndex_ = BindlessDescriptorSet::GetInstance().RegisterSampler(nearest_->GetVkHandle());
    compareIndex_ = BindlessDescriptorSet::GetInstance().RegisterCompareSampler(compare_->GetVkHandle());
}
