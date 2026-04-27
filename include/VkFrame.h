#pragma once
#include "MeshBuffer.h"
#include "RenderObject.h"
#include "Scene.h"
#include "VkObjects.h"
#include "Core/DeletionQueue.h"
#include "Core/RefCount.h"

class VkFrame {
public:
    VkFrame();

    ~VkFrame() = default;

    VkFrame(const VkFrame &) = delete;

    VkFrame &operator=(const VkFrame &) = delete;

    VkFrame(VkFrame &&) = default;

    VkFrame &operator=(VkFrame &&) = default;

    VkFence GetRenderFence() const {
        return renderFence_->GetVkHandle();
    }

    VkSemaphore GetImageAvailableSemaphore() const {
        return imageAvailableSemaphore_->GetVkHandle();
    }

    VkDescriptorSet Allocate(VkDescriptorSetLayout layout) {
        return descriptorPool_->Allocate(layout);
    }

    VkCommandBuffer BeginFrame();

    template<typename F>
    void ExecuteOnFrameEnd(F &&f) {
        deletionQueue_.PushFunction(std::forward<F>(f));
    }

    void EndFrame(VkSemaphoreSubmitInfo *signalInfo);

    RefCountedPtr<DescriptorPool> GetDescriptorPool() const {
        return descriptorPool_;
    }

    void WriteSceneData(const SceneData &sceneData);

    void WriteInvData(const InvData &mvpData);

    VkDeviceAddress GetSceneAddr() const {
        return sceneBuffer_->GetDeviceAddress();
    }

    VkDeviceAddress GetInvAddr() const {
        return invBuffer_->GetDeviceAddress();
    }

private:
    RefCountedPtr<Fence> renderFence_;
    RefCountedPtr<Semaphore> imageAvailableSemaphore_;
    RefCountedPtr<CommandPool> commandPool_;
    RefCountedPtr<CommandBuffer> commandBuffer_;
    RefCountedPtr<DescriptorPool> descriptorPool_;
    DeletionQueue deletionQueue_;

    RefCountedPtr<AllocatedBuffer> sceneBuffer_;
    RefCountedPtr<AllocatedBuffer> invBuffer_;
};
