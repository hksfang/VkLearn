#include "MeshBuffer.h"

#include <glm/gtx/hash.hpp>

#include "ImmediateContext.h"
#include "VkContext.h"


MeshBuffer::MeshBuffer(std::span<uint32_t> indices, std::span<Vertex> vertices) : vertexBuffer_(AllocatedBuffer::New(
            vertices.size() * sizeof(Vertex),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY)
    ),
    // TODO: Move staging / uploading part to allocated buffer instead.
    indexBuffer_(AllocatedBuffer::New(indices.size() * sizeof(uint32_t),
                                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                      VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                      VMA_MEMORY_USAGE_GPU_ONLY
    )),
    vertexCount_(vertices.size()),
    indexCount_(indices.size()) {
    vertexBufferAddress_ = vertexBuffer_->GetDeviceAddress();

    auto stagingBuffer = AllocatedBuffer::New(
        indices.size_bytes() + vertices.size_bytes(),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        // TODO: The tutorial says its CPU_ONLY, but shouldn't it be CPU_TO_GPU?
        VMA_MEMORY_USAGE_CPU_ONLY
    );

    // Write to mapped data.
    {
        void *data;
        VK_CHECK(vmaMapMemory(GetAllocator(), stagingBuffer->GetAllocation(), &data));
        memcpy(data, vertices.data(), vertices.size_bytes());
        memcpy((char *) data + vertices.size_bytes(), indices.data(), indices.size_bytes());
        VK_CHECK(vmaFlushAllocation(GetAllocator(), stagingBuffer->GetAllocation(),0, VK_WHOLE_SIZE));
        vmaUnmapMemory(GetAllocator(), stagingBuffer->GetAllocation());
    }

    ImmediateContext::GetInstance().ImmediateSubmit([&](VkCommandBuffer cmd) {
        VkBufferCopy vertexCopy{
            .srcOffset = 0,
            .dstOffset = 0,
            .size = vertices.size_bytes(),
        };
        vkCmdCopyBuffer(cmd, stagingBuffer->GetBuffer(), vertexBuffer_->GetBuffer(), 1, &vertexCopy);
        VkBufferCopy indexCopy{
            .srcOffset = vertices.size_bytes(),
            .dstOffset = 0,
            .size = indices.size_bytes(),
        };
        vkCmdCopyBuffer(cmd, stagingBuffer->GetBuffer(), indexBuffer_->GetBuffer(), 1, &indexCopy);
    });
}
