#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "RenderObject.h"
#include "Core/RefCount.h"

class MaterialInstance;
class MeshAsset;
class AllocatedImage;

class LoadedGLTF : public IRenderable, public RefCounted<LoadedGLTF> {
public:
    void Draw(const glm::mat4 &topMatrix, std::vector<RenderObject> &ctx) override;

protected:
    LoadedGLTF(const std::filesystem::path &filePath, GLTFMetallicRoughness *gltfMaterial,
               AllocatedImage *irradianceMap, AllocatedImage *radianceMap);

private:
    std::unordered_map<std::string, RefCountedPtr<MeshAsset> > meshes_;
    std::unordered_map<std::string, std::shared_ptr<RenderNode> > nodes_;
    std::vector<std::unique_ptr<AllocatedImage> > images_;
    std::unordered_map<std::string, RefCountedPtr<MaterialInstance> > materials_;

    std::vector<std::shared_ptr<RenderNode> > topNodes_;
    std::vector<RefCountedPtr<Sampler> > samplers_;
};
