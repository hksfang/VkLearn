#include "BindlessDescriptorSet.h"
#include "VkContext.h"
#include "VkInit.h"
#include <stdexcept>

void BindlessDescriptorSet::Init() {
    VkDescriptorSetLayoutBinding bindings[] = {
        {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = MAX_TEXTURES,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },
        {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = MAX_SAMPLERS,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },
        {
            .binding = 2,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = MAX_SAMPLERS,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },

        {
            .binding = 3,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = MAX_CUBEMAPS,
            .stageFlags = VK_SHADER_STAGE_ALL,
        }
    };

    VkDescriptorBindingFlags flags[] = {
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
    };

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = 4,
        .pBindingFlags = flags
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo = InitDescriptorSetLayoutCreateInfo(
        bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT);
    layoutInfo.pNext = &flagsInfo;

    layout_ = DescriptorSetLayout::New(layoutInfo);

    VkDescriptorPoolSize poolSizes[4] = {
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_TEXTURES},
        {VK_DESCRIPTOR_TYPE_SAMPLER, MAX_SAMPLERS},
        {VK_DESCRIPTOR_TYPE_SAMPLER, MAX_SAMPLERS},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_CUBEMAPS},
    };

    VkDescriptorPoolCreateInfo poolInfo = InitDescriptorPoolCreateInfo(poolSizes, 1,
                                                                       VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);
    pool_ = DescriptorPool::New(poolInfo);

    descriptorSet_ = pool_->Allocate(layout_->GetVkHandle());
}

uint32_t BindlessDescriptorSet::RegisterImage(VkImageView imageView) {
    uint32_t index = nextImageIndex_++;
    if (index >= MAX_TEXTURES) {
        throw std::runtime_error("Exceeded max bindless textures");
    }

    VkDescriptorImageInfo imageInfo = {
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet_,
        .dstBinding = 0,
        .dstArrayElement = index,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = &imageInfo
    };

    vkUpdateDescriptorSets(VulkanContext::GetInstance().GetDevice(), 1, &write, 0, nullptr);

    return index;
}

uint32_t BindlessDescriptorSet::RegisterTextureCube(VkImageView imageView) {
    uint32_t index = nextCubeIndex_++;
    if (index >= MAX_CUBEMAPS) {
        throw std::runtime_error("Exceeded max bindless cubemaps");
    }

    VkDescriptorImageInfo imageInfo = {
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet_,
        .dstBinding = 3,
        .dstArrayElement = index,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = &imageInfo
    };

    vkUpdateDescriptorSets(VulkanContext::GetInstance().GetDevice(), 1, &write, 0, nullptr);

    return index;
}

uint32_t BindlessDescriptorSet::RegisterSampler(VkSampler sampler) {
    uint32_t index = nextSamplerIndex_++;
    if (index >= MAX_SAMPLERS) {
        throw std::runtime_error("Exceeded max bindless samplers");
    }

    VkDescriptorImageInfo samplerInfo = {
        .sampler = sampler,
    };

    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet_,
        .dstBinding = 1,
        .dstArrayElement = index,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .pImageInfo = &samplerInfo
    };

    vkUpdateDescriptorSets(VulkanContext::GetInstance().GetDevice(), 1, &write, 0, nullptr);

    return index;
}

uint32_t BindlessDescriptorSet::RegisterCompareSampler(VkSampler sampler) {
    uint32_t index = nextCompareSamplerIndex_++;
    if (index >= MAX_SAMPLERS) {
        throw std::runtime_error("Exceeded max bindless samplers");
    }

    VkDescriptorImageInfo samplerInfo = {
        .sampler = sampler,
    };

    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet_,
        .dstBinding = 2,
        .dstArrayElement = index,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .pImageInfo = &samplerInfo
    };

    vkUpdateDescriptorSets(VulkanContext::GetInstance().GetDevice(), 1, &write, 0, nullptr);

    return index;
}
