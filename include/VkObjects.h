#pragma once
#include <filesystem>
#include <vector>

#include "VkRaii.h"
#include "Core/RefCount.h"

class Fence : public RefCounted<Fence>, public raii::Fence {
protected:
    Fence(const VkFenceCreateInfo &info) {
        Create(info);
    }
};

class Semaphore : public RefCounted<Semaphore>, public raii::Semaphore {
protected:
    Semaphore(const VkSemaphoreCreateInfo &info) {
        Create(info);
    }
};

class CommandPool : public RefCounted<CommandPool>, public raii::CommandPool {
protected:
    CommandPool(const VkCommandPoolCreateInfo &info) {
        Create(info);
    }
};

class Image : public RefCounted<Image>, public raii::Image {
protected:
    Image(const VkImageCreateInfo &info) {
        Create(info);
    }
};

class ImageView : public RefCounted<ImageView>, public raii::ImageView {
protected:
    ImageView(const VkImageViewCreateInfo &info) {
        Create(info);
    }
};

class CommandBuffer : public RefCounted<CommandBuffer>, public raii::CommandBuffer {
protected:
    CommandBuffer(CommandPool *commandPool, uint32_t count);

public:
    // Note: We are just freeing command pool instead.
    [[nodiscard]] const CommandPool *GetCommandPool() const { return commandPool_.Get(); }

private:
    const RefCountedPtr<CommandPool> commandPool_;
};

class Swapchain : public RefCounted<Swapchain>, public raii::Swapchain {
protected:
    Swapchain(uint32_t width, uint32_t height);

public:
    ~Swapchain() override;

    uint32_t AcquireNextImageIdx(VkSemaphore signalSemaphore);

    VkImage GetImage(uint32_t imageIdx) const;

    VkImageView GetImageView(uint32_t imageIdx) const;

    VkSemaphore GetSemaphoreForImageIdx(uint32_t imageIdx);

    VkExtent2D GetExtent() const {
        return vkSwapchainExtent_;
    }

    VkFormat GetFormat() const {
        return vkSwapchainFormat_;
    }

    const VkFormat *GetFormatAddr() const {
        return &vkSwapchainFormat_;
    }

    void Present(uint32_t imageIdx);

private:
    void Init(uint32_t width, uint32_t height);

    void DestroyCurrent();

    VkFormat vkSwapchainFormat_;
    VkExtent2D vkSwapchainExtent_;
    std::vector<VkImage> vkSwapchainImages_;
    std::vector<RefCountedPtr<Semaphore> > vkSwapchainImageSemaphores_;
    std::vector<VkImageView> vkSwapchainImageViews_;
};

class DescriptorSetLayout : public RefCounted<DescriptorSetLayout>, public raii::DescriptorSetLayout {
protected:
    DescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo &info) {
        Create(info);
    };
};

class DescriptorPool : public RefCounted<DescriptorPool>, public raii::DescriptorPool {
protected:
    DescriptorPool(const VkDescriptorPoolCreateInfo &info) {
        Create(info);
    }

public:
    VkDescriptorSet Allocate(VkDescriptorSetLayout layout);
};

class ShaderModule : public RefCounted<ShaderModule>, public raii::ShaderModule {
protected:
    ShaderModule(const std::filesystem::path &filePath);
};

class PipelineLayout : public RefCounted<PipelineLayout>, public raii::PipelineLayout {
protected:
    PipelineLayout(const VkPipelineLayoutCreateInfo &info) {
        Create(info);
    }
};

class ComputePipeline : public RefCounted<ComputePipeline>, public raii::ComputePipeline {
protected:
    ComputePipeline(const VkComputePipelineCreateInfo &info) {
        Create(info);
    }
};

class GraphicsPipeline : public RefCounted<GraphicsPipeline>, public raii::GraphicsPipeline {
protected:
    GraphicsPipeline(const VkGraphicsPipelineCreateInfo &info) {
        Create(info);
    }
};

class Sampler : public RefCounted<Sampler>, public raii::Sampler
{
    protected:
    Sampler(const VkSamplerCreateInfo &info)
    {
        Create(info);
    }
};