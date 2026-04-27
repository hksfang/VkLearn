#pragma once
#include "Material.h"
#include "MeshBuffer.h"

struct GeoSurface {
    GeoSurface(uint32_t startIndex, uint32_t count,
               RefCountedPtr<MaterialInstance> materialInstance) : startIndex_(startIndex),
                                                                   count_(count),
                                                                   material_(materialInstance) {
    }

    [[nodiscard]]
    RefCountedPtr<MaterialInstance> GetMaterial() const {
        return material_;
    }

    void SetMaterial(RefCountedPtr<MaterialInstance> material) {
        material_ = material;
    }

    [[nodiscard]] uint32_t GetCount() const {
        return count_;
    }

    [[nodiscard]] uint32_t GetStartIndex() const {
        return startIndex_;
    }

private:
    uint32_t startIndex_;
    uint32_t count_;
    RefCountedPtr<MaterialInstance> material_;
};

class MeshAsset : public RefCounted<MeshAsset> {
public:
    static RefCountedPtr<MeshAsset> CreateFromOBJFile(const std::filesystem::path &path);

    static std::vector<RefCountedPtr<MeshAsset> > CreateFromGLTFFile(const std::filesystem::path &path);

    ~MeshAsset() = default;

    const MeshBuffer &GetMeshBuffer() const {
        return meshBuffer_;
    }

    const std::string &GetName() const {
        return name_;
    }

    std::span<GeoSurface> GetSurfaces() {
        return geoSurfaces_;
    }

protected:
    MeshAsset(std::span<Vertex> vertices, std::span<uint32_t> indices, std::span<GeoSurface> geoSurfaces,
              std::string_view name) : meshBuffer_(indices, vertices), name_(name),
                                       geoSurfaces_(geoSurfaces.begin(), geoSurfaces.end()) {
    }

private:
    MeshBuffer meshBuffer_;
    std::string name_;
    std::vector<GeoSurface> geoSurfaces_;
};
