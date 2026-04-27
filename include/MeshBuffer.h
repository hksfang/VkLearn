#pragma once
#include <span>
#include <glm/glm.hpp>

#include "AllocatedBuffer.h"

struct Vertex {
    glm::vec3 position;
    float uv_x;
    glm::vec3 normal;
    float uv_y;
    glm::vec4 color;
    glm::vec4 tangent;

    bool operator==(const Vertex &other) const {
        return position == other.position && color == other.color &&
               uv_x == other.uv_x && uv_y == other.uv_y && normal == other.normal && tangent == other.tangent;
    }
};


class MeshBuffer {
public:
    MeshBuffer(std::span<uint32_t> indices, std::span<Vertex> vertices);

    ~MeshBuffer() = default;

    const RefCountedPtr<AllocatedBuffer> GetIndexBuffer() const { return indexBuffer_; }

    VkDeviceAddress GetBufferAddress() const {
        return vertexBufferAddress_;
    }

    uint32_t GetIndexCount() const { return indexCount_; }

private:
    RefCountedPtr<AllocatedBuffer> vertexBuffer_;
    RefCountedPtr<AllocatedBuffer> indexBuffer_;
    MoveOnly<VkDeviceAddress> vertexBufferAddress_{};
    uint32_t vertexCount_{};
    uint32_t indexCount_{};
};

struct InvData {
    // Cached inv.
    glm::mat4 invModelMatrix;
    glm::mat4 invViewMatrix;
    glm::mat4 invProjMatrix;
};

struct DrawPushConstants {
    glm::mat4 modelMatrix;
    VkDeviceAddress invData;
    VkDeviceAddress vertexBuffer;
    VkDeviceAddress materialDataBuffer;
    VkDeviceAddress sceneDataBuffer;
};
