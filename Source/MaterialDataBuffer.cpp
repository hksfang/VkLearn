#include "MaterialDataBuffer.h"

MaterialDataBuffer::MaterialDataBuffer() {
    materialDataBuffer_ = AllocatedBuffer::New(
        MAX_MATERIAL_DATA_SIZE,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU
    );
}
