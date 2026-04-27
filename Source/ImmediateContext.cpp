#include "ImmediateContext.h"

#include "VkContext.h"
#include "VkInit.h"

ImmediateContext::ImmediateContext() : fence_(Fence::New(InitFenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT))),
                                       commandPool_(CommandPool::New(InitCommandPoolCreateInfo(
                                           VulkanContext::GetInstance().GetGraphicsQueueFamilyIndex(),
                                           VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT))),
                                       commandBuffer_(
                                           CommandBuffer::New(commandPool_.Get(), 1)) {
    // Make sure VulkanContext is constructed before Immediate Context.
    VulkanContext::GetInstance();
}


