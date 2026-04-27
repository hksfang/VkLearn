#pragma once
#include <cstring>

#include "AllocatedBuffer.h"
#include "Core/Singleton.h"


class MaterialDataBuffer : public Singleton<MaterialDataBuffer> {
public:
    template<typename T>
    uint32_t AppendMaterial(const T &data) {
        uint32_t size = sizeof(T);
        uint32_t alignedSize = (size + 15) & ~15;
        uint32_t currentOffset = currentFreeOffset_;

        if (currentOffset + alignedSize > materialDataBuffer_->GetSize()) {
            return -1;
        }

        void *mapped = materialDataBuffer_->GetMappedData<void>();
        memcpy(static_cast<uint8_t *>(mapped) + currentOffset, &data, size);

        currentFreeOffset_ += alignedSize;
        return currentOffset;
    }

    RefCountedPtr<AllocatedBuffer> GetBuffer() const {
        return materialDataBuffer_;
    }

    VkDescriptorBufferInfo GetDescriptorInfo() const {
        return {
            .buffer = materialDataBuffer_->GetBuffer(),
            .offset = 0,
            .range = VK_WHOLE_SIZE
        };
    }

protected:
    MaterialDataBuffer();

private:
    RefCountedPtr<AllocatedBuffer> materialDataBuffer_;
    uint32_t currentFreeOffset_{0};

    static constexpr size_t MAX_MATERIAL_DATA_SIZE = 1024 * 1024 * 16; // 16MB
};
