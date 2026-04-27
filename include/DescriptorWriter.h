#pragma once
#include <deque>
#include <vector>
#include <vulkan/vulkan_core.h>

class DescriptorWriter {
public:
    DescriptorWriter() = default;

    ~DescriptorWriter() = default;

    void WriteImage(int binding,
                    VkImageView imageView,
                    VkSampler sampler,
                    VkImageLayout imageLayout,
                    VkDescriptorType type);

    void WriteBuffer(int binding,
                     VkBuffer buffer,
                     size_t size,
                     size_t offset,
                     VkDescriptorType type);

    void Clear() {
        imageInfos_.clear();
        bufferInfos_.clear();
        writeDescriptorSet_.clear();
    }

    void UpdateSet(VkDescriptorSet set);

private:
    std::deque<VkDescriptorImageInfo> imageInfos_;
    std::deque<VkDescriptorBufferInfo> bufferInfos_;
    std::vector<VkWriteDescriptorSet> writeDescriptorSet_;
};
