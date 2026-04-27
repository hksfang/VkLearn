#include "VkFrame.h"

#include "VkContext.h"
#include "VkDebug.h"
#include "VkInit.h"

VkFrame::VkFrame() : renderFence_(Fence::New(InitFenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT))),
                     imageAvailableSemaphore_(Semaphore::New(InitSemaphoreCreateInfo())),
                     commandPool_(CommandPool::New(InitCommandPoolCreateInfo(
                         VulkanContext::GetInstance().GetGraphicsQueueFamilyIndex(),
                         VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT))),
                     commandBuffer_(
                         CommandBuffer::New(commandPool_.Get(), 1)),
                     sceneBuffer_(AllocatedBuffer::New(sizeof(SceneData),
                                                       VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                       VMA_MEMORY_USAGE_CPU_TO_GPU)),
                     invBuffer_(AllocatedBuffer::New(sizeof(InvData),
                                                     VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                                     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                     VMA_MEMORY_USAGE_CPU_TO_GPU)
                     ) {
    // Init descriptor pool.
    static constexpr VkDescriptorPoolSize poolSizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1024},
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1024},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024},
    };
    descriptorPool_ = DescriptorPool::New(InitDescriptorPoolCreateInfo(poolSizes, 1024, 0));
}

VkCommandBuffer VkFrame::BeginFrame() {
    auto vkDevice = VulkanContext::GetInstance().GetDevice();
    VK_CHECK(vkWaitForFences(vkDevice, 1, renderFence_->GetVkHandleAddr(), true, 1000000000));
    deletionQueue_.Flush();
    vkResetDescriptorPool(GetDevice(), descriptorPool_->GetVkHandle(), 0);
    VK_CHECK(vkResetFences(vkDevice, 1, renderFence_->GetVkHandleAddr()));

    VK_CHECK(vkResetCommandBuffer(commandBuffer_->GetVkHandle(), 0));
    auto cmdBeginInfo = InitCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VK_CHECK(vkBeginCommandBuffer(commandBuffer_->GetVkHandle(), &cmdBeginInfo));

    return commandBuffer_->GetVkHandle();
}

void VkFrame::EndFrame(VkSemaphoreSubmitInfo *signalInfo) {
    // Submit the command buffer.
    auto cmdBufferSubmitInfo = InitCommandBufferSubmitInfo(commandBuffer_->GetVkHandle());
    auto waitInfo = InitSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                                            GetImageAvailableSemaphore());
    auto submitInfo = InitSubmitInfo(&cmdBufferSubmitInfo, signalInfo, &waitInfo);
    auto vkGraphicsQueue = VulkanContext::GetInstance().GetGraphicsQueue();
    VK_CHECK(vkQueueSubmit2(vkGraphicsQueue, 1, &submitInfo, GetRenderFence()));
}

void VkFrame::WriteSceneData(const SceneData &sceneData) {
    SceneData *mapped = sceneBuffer_->GetMappedData<SceneData>();
    *mapped = sceneData;
}

void VkFrame::WriteInvData(const InvData &mvpData) {
    InvData *mapped = invBuffer_->GetMappedData<InvData>();
    *mapped = mvpData;
}
