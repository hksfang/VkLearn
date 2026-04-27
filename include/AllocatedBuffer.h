#pragma once

#include <vulkan/vulkan.h>

#include "AllocatedImage.h"

class AllocatedBuffer : public RefCounted<AllocatedBuffer> {
protected:
    AllocatedBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

public:
    ~AllocatedBuffer();

    VkBuffer GetBuffer() const {
        return buffer_;
    }

    VmaAllocation GetAllocation() {
        return allocation_;
    }

    template<typename T>
    T *GetMappedData() {
        return static_cast<T *>(allocationInfo_.pMappedData);
    }

    size_t GetSize() const {
        return allocationInfo_.size;
    }

    VkDeviceAddress GetDeviceAddress() const {
        return deviceAddress_;
    }

private:
    VkBuffer buffer_;
    VmaAllocation allocation_;
    VmaAllocationInfo allocationInfo_;
    VkDeviceAddress deviceAddress_;
};
