#include "MeshAsset.h"

#include <tiny_obj_loader.h>
#include <spdlog/spdlog.h>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <stb_image.h>

#include <glm/gtx/hash.hpp>

#include "Core/Debug.h"

namespace std {
    template<>
    struct hash<Vertex> {
        size_t operator()(Vertex const &vertex) const noexcept {
            size_t seed = 0;

            auto hash_combine = [&](size_t &seed, size_t v) {
                seed ^= v + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            };

            hash_combine(seed, std::hash<glm::vec3>()(vertex.position));
            hash_combine(seed, std::hash<glm::vec3>()(vertex.normal));
            hash_combine(seed, std::hash<glm::vec2>()({vertex.uv_x, vertex.uv_y}));
            hash_combine(seed, std::hash<glm::vec3>()(vertex.color));
            return seed;
        }
    };
}


RefCountedPtr<MeshAsset> MeshAsset::CreateFromOBJFile(const std::filesystem::path &objFilePath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    CHECK(std::filesystem::exists(objFilePath));
    const auto &parentPath = objFilePath.parent_path();
    std::string warn;
    std::string err;
    CHECK(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objFilePath.string().c_str(), parentPath.string().
        c_str()
    ));
    if (!warn.empty()) {
        spdlog::warn("[TinyObjLoader] WARN: {}", warn);
    }
    if (!err.empty()) {
        spdlog::error("[TinyObjLoader] ERROR: {}", err);
    }

    spdlog::info("[TinyObjLoader] # of vertices: {}", attrib.vertices.size() / 3);
    spdlog::info("[TinyObjLoader] # of normals: {}", attrib.normals.size() / 3);
    spdlog::info("[TinyObjLoader] # of texcoords: {}", attrib.texcoords.size() / 3);
    spdlog::info("[TinyObjLoader] # of materials: {}", materials.size());
    spdlog::info("[TinyObjLoader] # of shapes: {}", shapes.size());

    // TODO: Load diffuse textures
    std::unordered_map<Vertex, uint32_t> uniqueVertices;

    glm::vec4 defaultColor{1.0f, 0.0f, 0.0f, 1.0f};

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<GeoSurface> geoSurfaces;
    int mtlId = 0;
    for (const auto &shape: shapes) {
        int startIndex = indices.size();
        // faces
        for (int f = 0; f < shape.mesh.indices.size(); ++f) {
            auto &index = shape.mesh.indices[f];
            mtlId = shape.mesh.material_ids[f / 3];
            Vertex vertex{};

            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            if (index.texcoord_index >= 0) {
                vertex.uv_x =
                        attrib.texcoords[2 * index.texcoord_index + 0];
                vertex.uv_y = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];
            };

            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }
            vertex.color = defaultColor;

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
        // Load diffuse
        // const auto &texturePath = parentPath / materials[mtlId].diffuse_texname;
        // const auto &specularTexturePath = parentPath / materials[mtlId].specular_texname;

        geoSurfaces.emplace_back(
            static_cast<uint32_t>(startIndex),
            static_cast<uint32_t>(indices.size() - startIndex),
            nullptr
        );
    }
    return New(vertices, indices, geoSurfaces, "");
}

std::vector<RefCountedPtr<MeshAsset> > MeshAsset::CreateFromGLTFFile(const std::filesystem::path &path) {
    CHECK(std::filesystem::exists(path));
    spdlog::info("Loading GLTF Mesh Asset from {}", path.string());
    fastgltf::GltfDataBuffer dataBuffer;
    auto data = fastgltf::GltfDataBuffer::FromPath(path);
    CHECK(data);
    dataBuffer = std::move(data.get());

    const auto gltfOptions = fastgltf::Options::LoadExternalBuffers;
    fastgltf::Asset gltf;
    fastgltf::Parser parser{};

    auto load = parser.loadGltfBinary(dataBuffer, path.parent_path(), gltfOptions);
    CHECK(load);
    gltf = std::move(load.get());

    std::vector<RefCountedPtr<MeshAsset> > meshAssets;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<GeoSurface> geoSurfaces;

    spdlog::info("[fastgltf] # of meshes: {}", gltf.meshes.size());
    for (auto &mesh: gltf.meshes) {
        vertices.clear();
        indices.clear();
        geoSurfaces.clear();
        for (auto &p: mesh.primitives) {
            geoSurfaces.emplace_back(
                static_cast<uint32_t>(indices.size()),
                static_cast<uint32_t>(gltf.accessors[p.indicesAccessor.value()].count),
                nullptr);

            auto initialVtx = vertices.size();
            // Load indexes
            auto &indexAccessor = gltf.accessors[p.indicesAccessor.value()];
            fastgltf::iterateAccessor<uint32_t>(gltf, indexAccessor, [&](auto idx) {
                indices.emplace_back(static_cast<uint32_t>(idx + initialVtx));
            });
            // Load vertices
            auto &positionAccessor = gltf.accessors[p.findAttribute("POSITION")->accessorIndex];
            vertices.resize(vertices.size() + positionAccessor.count);
            fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, positionAccessor, [&](auto pos, auto idx) {
                Vertex vtx{
                    .position = pos
                };
                vertices[initialVtx + idx] = vtx;
            });
            auto normals = p.findAttribute("NORMAL");
            if (normals != p.attributes.end()) {
                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[normals->accessorIndex],
                                                              [&](glm::vec3 v, size_t index) {
                                                                  vertices[initialVtx + index].normal = v;
                                                              });
            }
            auto uv = p.findAttribute("TEXCOORD_0");
            if (uv != p.attributes.end()) {
                fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[uv->accessorIndex],
                                                              [&](glm::vec2 v, size_t index) {
                                                                  vertices[initialVtx + index].uv_x = v.x;
                                                                  vertices[initialVtx + index].uv_y = v.y;
                                                              });
            }
            auto colors = p.findAttribute("COLOR_0");
            if (colors != p.attributes.end()) {
                fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[colors->accessorIndex],
                                                              [&](glm::vec4 v, size_t index) {
                                                                  vertices[initialVtx + index].color = v;
                                                              });
            }
        }
        constexpr bool OverrideColors = true;
        if (OverrideColors) {
            for (Vertex &vtx: vertices) {
                vtx.color = glm::vec4(vtx.normal, 1.f);
            }
        }
        meshAssets.emplace_back(New(vertices, indices, geoSurfaces, mesh.name));
    }
    return meshAssets;
}
