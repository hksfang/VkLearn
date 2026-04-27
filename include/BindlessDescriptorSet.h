#pragma once
#include <vulkan/vulkan.h>
#include "Core/Singleton.h"
#include "VkObjects.h"

class BindlessDescriptorSet : public Singleton<BindlessDescriptorSet> {
public:
    uint32_t RegisterImage(VkImageView imageView);

    uint32_t RegisterTextureCube(VkImageView imageView);

    uint32_t RegisterSampler(VkSampler sampler);

    uint32_t RegisterCompareSampler(VkSampler sampler);

    VkDescriptorSet GetSet() const { return descriptorSet_; }
    VkDescriptorSetLayout GetLayout() const { return layout_->GetVkHandle(); }

protected:
    BindlessDescriptorSet() {
        Init();
    }

private:
    void Init();

    RefCountedPtr<DescriptorSetLayout> layout_;
    RefCountedPtr<DescriptorPool> pool_;
    VkDescriptorSet descriptorSet_ = VK_NULL_HANDLE;

    uint32_t nextImageIndex_ = 0;
    uint32_t nextSamplerIndex_ = 0;
    uint32_t nextCubeIndex_ = 0;
    uint32_t nextCompareSamplerIndex_ = 0;

    static constexpr uint32_t MAX_TEXTURES = 10000;
    static constexpr uint32_t MAX_SAMPLERS = 1000;
    static constexpr uint32_t MAX_CUBEMAPS = 100;
};
