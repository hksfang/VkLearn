#pragma once
#include "VkContext.h"
#include "VkDebug.h"
#include "VkInit.h"
#include "VkObjects.h"

class ImmediateContext : public Singleton<ImmediateContext> {
protected:
    ImmediateContext();

public:
    ~ImmediateContext() = default;

    template<typename Callable>
    void ImmediateSubmit(Callable func);

private:
    RefCountedPtr<Fence> fence_{nullptr};
    RefCountedPtr<CommandPool> commandPool_{nullptr};
    RefCountedPtr<CommandBuffer> commandBuffer_{nullptr};
};

template<typename Callable>
void ImmediateContext::ImmediateSubmit(Callable func) {
    VK_CHECK(vkResetFences(GetDevice(), 1, fence_->GetVkHandleAddr()));
    VK_CHECK(vkResetCommandBuffer(commandBuffer_->GetVkHandle(), 0));

    // Begin CommandBuffer
    auto cmd = commandBuffer_->GetVkHandle();
    auto beginInfo = InitCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));
    func(cmd);
    VK_CHECK(vkEndCommandBuffer(cmd));
    // Submit
    auto cmdInfo = InitCommandBufferSubmitInfo(cmd);
    auto submitInfo = InitSubmitInfo(&cmdInfo, nullptr, nullptr);

    VK_CHECK(vkQueueSubmit2(GetGraphicsQueue(), 1, &submitInfo, fence_->GetVkHandle()));
    VK_CHECK(vkWaitForFences(GetDevice(), 1, fence_->GetVkHandleAddr(), VK_TRUE, UINT64_MAX));
}
