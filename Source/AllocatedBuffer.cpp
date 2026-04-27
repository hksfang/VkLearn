#include "AllocatedBuffer.h"

#include "VkContext.h"
#include "VkDebug.h"
#include "VkInit.h"

AllocatedBuffer::AllocatedBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
    auto bufferCreateInfo = InitBufferCreateInfo(allocSize, usage);
    auto vmaallocInfo = InitVmaAllocationCreateInfo(memoryUsage, 0, VMA_ALLOCATION_CREATE_MAPPED_BIT);
    VK_CHECK(
        vmaCreateBuffer(GetAllocator(), &bufferCreateInfo, &vmaallocInfo, &buffer_, &allocation_, &allocationInfo_));

    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        VkBufferDeviceAddressInfo info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = buffer_
        };
        deviceAddress_ = vkGetBufferDeviceAddress(GetDevice(), &info);
    } else {
        deviceAddress_ = 0;
    }
}

AllocatedBuffer::~AllocatedBuffer() {
    vmaDestroyBuffer(GetAllocator(), buffer_, allocation_);
}
