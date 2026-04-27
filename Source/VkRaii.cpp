#include <VkDebug.h>
#include "VkRaii.h"

#include <VkBootstrap.h>

#include "VkContext.h"


namespace raii
{
#define IMPL_RAII_SIMPLE(Type) \
    void Type::Create(const Vk##Type##CreateInfo& createInfo) { \
        VK_CHECK(vkCreate##Type(GetDevice(), &createInfo, nullptr, &handle_)); \
        } \
    Type::~Type() \
    { \
        vkDestroy##Type(GetDevice(), handle_, nullptr); \
    }
    IMPL_RAII_SIMPLE(CommandPool);
    IMPL_RAII_SIMPLE(Fence);
    IMPL_RAII_SIMPLE(Image);
    IMPL_RAII_SIMPLE(ImageView);
    IMPL_RAII_SIMPLE(DescriptorSetLayout);
    IMPL_RAII_SIMPLE(DescriptorPool);
    IMPL_RAII_SIMPLE(ShaderModule);
    IMPL_RAII_SIMPLE(Semaphore);
    IMPL_RAII_SIMPLE(PipelineLayout);
    IMPL_RAII_SIMPLE(Buffer);
    IMPL_RAII_SIMPLE(Sampler);

    void CommandBuffer::Create(const VkCommandBufferAllocateInfo& info)
    {
        VK_CHECK(vkAllocateCommandBuffers(GetDevice(), &info, &handle_));
    }

    CommandBuffer::~CommandBuffer() = default;

    void Swapchain::Create(const VkSwapchainKHR& swapchain)
    {
        handle_ = swapchain;
    }

    Swapchain::~Swapchain()
    {
        vkDestroySwapchainKHR(GetDevice(), handle_, nullptr);
    }

    void ComputePipeline::Create(const VkComputePipelineCreateInfo& info)
    {
        VK_CHECK(vkCreateComputePipelines(GetDevice(), VK_NULL_HANDLE, 1, &info, nullptr, &handle_));
    }

    ComputePipeline::~ComputePipeline()
    {
        vkDestroyPipeline(GetDevice(), handle_, nullptr);
    }

    void GraphicsPipeline::Create(const VkGraphicsPipelineCreateInfo& info)
    {
        VK_CHECK(vkCreateGraphicsPipelines(GetDevice(), nullptr, 1, &info, nullptr, &handle_));
    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        vkDestroyPipeline(GetDevice(), handle_, nullptr);
    }
}
