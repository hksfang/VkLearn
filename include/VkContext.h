#pragma once
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

#include "Core/Singleton.h"

// TODO: Make each element of VulkanContext ref counted instead.
class VulkanContext : public Singleton<VulkanContext> {
public:
    VkInstance GetVkInstance() const {
        return instance_;
    }

    VkPhysicalDevice GetPhysicalDevice() const {
        return physicalDevice_;
    }

    VkDevice GetDevice() const {
        return device_;
    }

    VkSurfaceKHR GetSurface() const {
        return surface_;
    }

    VkQueue GetGraphicsQueue() const {
        return graphicsQueue_;
    }

    uint32_t GetGraphicsQueueFamilyIndex() const {
        return graphicsQueueFamilyIndex_;
    }

    VmaAllocator GetVMAAllocator() const {
        return allocator_;
    }

    void DrainDevice() const;

protected:
    VulkanContext();

    ~VulkanContext();

private:
    VkDevice device_ = VK_NULL_HANDLE;
    VkInstance instance_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;
    VkQueue graphicsQueue_ = VK_NULL_HANDLE;
    uint32_t graphicsQueueFamilyIndex_ = 0;
    VmaAllocator allocator_ = VK_NULL_HANDLE;
};

VkInstance GetInstance();
VkPhysicalDevice GetPhysicalDevice();
VkDevice GetDevice();
VmaAllocator GetAllocator();
VkQueue GetGraphicsQueue();
uint32_t GetGraphicsQueueFamilyIndex();