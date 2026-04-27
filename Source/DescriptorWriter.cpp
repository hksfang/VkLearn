#include <DescriptorWriter.h>

#include "VkContext.h"


void DescriptorWriter::WriteImage(int binding, VkImageView imageView, VkSampler sampler, VkImageLayout imageLayout,
                                  VkDescriptorType type) {
    VkDescriptorImageInfo &info = imageInfos_.emplace_back(
        VkDescriptorImageInfo{
            .sampler = sampler,
            .imageView = imageView,
            .imageLayout = imageLayout,
        }
    );
    writeDescriptorSet_.push_back(
        VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = VK_NULL_HANDLE,
            .dstBinding = static_cast<uint32_t>(binding),
            .descriptorCount = 1,
            .descriptorType = type,
            .pImageInfo = &info
        }
    );
}

void DescriptorWriter::WriteBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type) {
    VkDescriptorBufferInfo &info = bufferInfos_.emplace_back(
        VkDescriptorBufferInfo{
            .buffer = buffer,
            .offset = offset,
            .range = size
        }
    );

    writeDescriptorSet_.emplace_back(VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = static_cast<uint32_t>(binding),
        .descriptorCount = 1,
        .descriptorType = type,
        .pBufferInfo = &info
    });
}

void DescriptorWriter::UpdateSet(VkDescriptorSet set) {
    for (auto &write: writeDescriptorSet_) {
        write.dstSet = set;
    }
    vkUpdateDescriptorSets(
        GetDevice(),
        writeDescriptorSet_.size(),
        writeDescriptorSet_.data(),
        0,
        nullptr);
}
