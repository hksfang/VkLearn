#pragma once

#include <glm/glm.hpp>

#include "MeshAsset.h"


class RenderObject;

class IRenderable {
public:
    virtual ~IRenderable() = default;

private:
    virtual void Draw(const glm::mat4 &topMatrix, std::vector<RenderObject> &ctx) = 0;
};

class RenderNode : public IRenderable {
public:
    RenderNode(const glm::mat4 &localTransform, const glm::mat4 &worldTransform) : localTransform_(localTransform),
        worldTransform_(worldTransform) {
    }

    void UpdateTransform(const glm::mat4 &parentMatrix) {
        worldTransform_ = parentMatrix * localTransform_;
        for (auto &c: children_) {
            c->UpdateTransform(worldTransform_);
        }
    }

    void Draw(const glm::mat4 &topMatrix, std::vector<RenderObject> &ctx) override {
        for (auto &c: children_) {
            c->Draw(topMatrix, ctx);
        }
    }

    void AddChild(std::shared_ptr<RenderNode> child) {
        children_.push_back(child);
    }

    void SetParent(std::shared_ptr<RenderNode> parent) {
        parent_ = parent;
    }

    std::shared_ptr<RenderNode> GetParent() {
        return parent_.lock();
    }

protected:
    glm::mat4 localTransform_;
    glm::mat4 worldTransform_;

private:
    std::weak_ptr<RenderNode> parent_;
    std::vector<std::shared_ptr<RenderNode> > children_;
};

class MeshNode : public RenderNode {
public:
    MeshNode(const glm::mat4 &localTransform, const glm::mat4 &worldTransform,
             RefCountedPtr<MeshAsset> meshAsset) : RenderNode(localTransform, worldTransform),
                                                   meshAsset_(meshAsset) {
    }

    void Draw(const glm::mat4 &topMatrix, std::vector<RenderObject> &ctx) override;

    auto GetMeshSurfaces() {
        return meshAsset_->GetSurfaces();
    }

private:
    RefCountedPtr<MeshAsset> meshAsset_;
};

class RenderObject {
public:
    RenderObject(uint32_t indexCount, uint32_t firstIndex, VkBuffer indexBuffer, const glm::mat4 &modelMatrix,
                 VkDeviceAddress deviceAddress,
                 const RefCountedPtr<MaterialInstance> &materialInstance) : indexCount_(indexCount),
                                                                            firstIndex_(firstIndex),
                                                                            indexBuffer_(indexBuffer),
                                                                            vertexBufferAddr_(deviceAddress),
                                                                            modelMatrix_(modelMatrix),
                                                                            materialInstance_(materialInstance) {
    }

    const glm::mat4& GetModelMatrix() const {
        return modelMatrix_;
    }

    const RefCountedPtr<MaterialInstance>& GetMaterialInstance() const {
        return materialInstance_;
    }

    void Draw(VkCommandBuffer cmd, VkDeviceAddress invDataAddr, VkDeviceAddress sceneDataAddr);

private:
    uint32_t indexCount_;
    uint32_t firstIndex_;
    VkBuffer indexBuffer_;
    VkDeviceAddress vertexBufferAddr_;
    glm::mat4 modelMatrix_;
    RefCountedPtr<MaterialInstance> materialInstance_;
};
