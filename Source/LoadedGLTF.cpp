#include "LoadedGLTF.h"

#include <stb_image.h>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <glm/detail/type_quat.hpp>
#include <glm/gtc/quaternion.hpp>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "BindlessDescriptorSet.h"
#include "DefaultTextures.h"
#include "VkInit.h"
#include "VkContext.h"
#include "Core/Debug.h"
#include "MeshAsset.h"

namespace {
    VkFilter GltfFilterToVkFilter(fastgltf::Filter filter) {
        switch (filter) {
            case fastgltf::Filter::Nearest:
            case fastgltf::Filter::NearestMipMapNearest:
            case fastgltf::Filter::NearestMipMapLinear:
                return VK_FILTER_NEAREST;
                break;
            case fastgltf::Filter::Linear:
            case fastgltf::Filter::LinearMipMapNearest:
            case fastgltf::Filter::LinearMipMapLinear:
                return VK_FILTER_LINEAR;
                break;
            default:
                return VK_FILTER_NEAREST;
        }
    }

    VkSamplerMipmapMode ExtractMipMapMode(fastgltf::Filter filter) {
        switch (filter) {
            case fastgltf::Filter::Nearest:
            case fastgltf::Filter::NearestMipMapNearest:
            case fastgltf::Filter::NearestMipMapLinear:
                return VK_SAMPLER_MIPMAP_MODE_NEAREST;
                break;
            case fastgltf::Filter::LinearMipMapNearest:
            case fastgltf::Filter::Linear:
            case fastgltf::Filter::LinearMipMapLinear:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
                break;
            default:
                return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        }
    }

    std::unique_ptr<AllocatedImage> LoadImage(fastgltf::Asset &asset, fastgltf::Image &image, VkFormat format) {
        std::unique_ptr<AllocatedImage> allocatedImage;
        int width, height, nrChannels;

        std::visit(fastgltf::visitor{
                       [](auto &arg) {
                           spdlog::error("Unhanded image type.");
                       },
                       [&](fastgltf::sources::URI &uri) {
                           CHECK(uri.fileByteOffset == 0);
                           CHECK(uri.uri.isLocalPath());
                           auto *data = stbi_load(uri.uri.c_str(), &width, &height, &nrChannels, 4);
                           CHECK(data);
                           VkExtent3D extent{
                               .width = static_cast<uint32_t>(width),
                               .height = static_cast<uint32_t>(height),
                               .depth = 1
                           };

                           allocatedImage = std::make_unique<AllocatedImage>(
                               data, extent, 4, format, 0, 1,
                               VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                               true);
                           stbi_image_free(data);
                       },
                       [&](fastgltf::sources::Vector &vector) {
                           auto *data = stbi_load_from_memory(reinterpret_cast<unsigned char *>(vector.bytes.data()),
                                                              vector.bytes.size(), &width, &height,
                                                              &nrChannels, 4);
                           CHECK(data);
                           VkExtent3D extent{
                               .width = static_cast<uint32_t>(width),
                               .height = static_cast<uint32_t>(height),
                               .depth = 1
                           };
                           allocatedImage = std::make_unique<AllocatedImage>(
                               data, extent, 4, format, 0, 1,
                               VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                               true);
                           stbi_image_free(data);
                       },
                       [&](fastgltf::sources::BufferView &view) {
                           auto &bufferView = asset.bufferViews[view.bufferViewIndex];
                           auto &buffer = asset.buffers[bufferView.bufferIndex];
                           std::visit(fastgltf::visitor{
                                          [](auto &arg) {
                                              spdlog::error("Unsupported image arg type!");
                                          },
                                          [&](fastgltf::sources::Array &array) {
                                              auto *data = stbi_load_from_memory(reinterpret_cast<stbi_uc const *>(
                                                      array.bytes.data() + bufferView.byteOffset),
                                                  static_cast<int>(bufferView.byteLength)
                                                  , &width, &height, &nrChannels, 4);
                                              CHECK(data);
                                              VkExtent3D extent{
                                                  .width = static_cast<uint32_t>(width),
                                                  .height = static_cast<uint32_t>(height),
                                                  .depth = 1
                                              };
                                              allocatedImage = std::make_unique<AllocatedImage>(
                                                  data, extent, 4, format, 0, 1,
                                                  VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                  true);
                                              stbi_image_free(data);
                                          },
                                          [&](fastgltf::sources::Vector &vector) {
                                              auto *data = stbi_load_from_memory(
                                                  reinterpret_cast<stbi_uc const *>(
                                                      vector.bytes.data() + bufferView.byteOffset),
                                                  static_cast<int>(bufferView.byteLength),
                                                  &width, &height, &nrChannels, 4);
                                              CHECK(data);
                                              VkExtent3D extent{
                                                  .width = static_cast<uint32_t>(width),
                                                  .height = static_cast<uint32_t>(height),
                                                  .depth = 1
                                              };
                                              allocatedImage = std::make_unique<AllocatedImage>(
                                                  data, extent, 4, format, 0, 1,
                                                  VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                  true);
                                              stbi_image_free(data);
                                          }
                                      }, buffer.data);
                       }
                   },
                   image.data
        );
        if (!allocatedImage) {
            spdlog::error("Allocated image is null!");
        }
        return allocatedImage;
    }

    void CalculateTangents(std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices,
                           uint32_t indexStart, uint32_t indexCount,
                           uint32_t vertexStart, uint32_t vertexCount) {
        if (indexCount == 0) return;

        std::vector<glm::vec3> tan1(vertexCount, glm::vec3(0.0f));
        std::vector<glm::vec3> tan2(vertexCount, glm::vec3(0.0f));

        for (uint32_t i = 0; i < indexCount; i += 3) {
            uint32_t a = indices[indexStart + i];
            uint32_t b = indices[indexStart + i + 1];
            uint32_t c = indices[indexStart + i + 2];

            uint32_t la = a - vertexStart;
            uint32_t lb = b - vertexStart;
            uint32_t lc = c - vertexStart;

            const glm::vec3 &v1 = vertices[a].position;
            const glm::vec3 &v2 = vertices[b].position;
            const glm::vec3 &v3 = vertices[c].position;

            const glm::vec2 w1(vertices[a].uv_x, vertices[a].uv_y);
            const glm::vec2 w2(vertices[b].uv_x, vertices[b].uv_y);
            const glm::vec2 w3(vertices[c].uv_x, vertices[c].uv_y);

            float x1 = v2.x - v1.x;
            float x2 = v3.x - v1.x;
            float y1 = v2.y - v1.y;
            float y2 = v3.y - v1.y;
            float z1 = v2.z - v1.z;
            float z2 = v3.z - v1.z;

            float s1 = w2.x - w1.x;
            float s2 = w3.x - w1.x;
            float t1 = w2.y - w1.y;
            float t2 = w3.y - w1.y;

            float div = (s1 * t2 - s2 * t1);
            float r = (std::abs(div) < 1e-6f) ? 0.0f : 1.0f / div;
            glm::vec3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
                           (t2 * z1 - t1 * z2) * r);
            glm::vec3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
                           (s1 * z2 - s2 * z1) * r);

            tan1[la] += sdir;
            tan1[lb] += sdir;
            tan1[lc] += sdir;

            tan2[la] += tdir;
            tan2[lb] += tdir;
            tan2[lc] += tdir;
        }

        for (uint32_t i = 0; i < vertexCount; ++i) {
            uint32_t idx = vertexStart + i;
            const glm::vec3 &n = vertices[idx].normal;
            const glm::vec3 &t = tan1[i];

            if (glm::length(t) > 1e-6f) {
                glm::vec3 tangent = glm::normalize(t - n * glm::dot(n, t));
                float w = (glm::dot(glm::cross(n, t), tan2[i]) < 0.0f) ? -1.0f : 1.0f;
                vertices[idx].tangent = glm::vec4(tangent, w);
            } else {
                glm::vec3 tangent;
                if (std::abs(n.x) > 0.1f) tangent = glm::normalize(glm::cross(n, glm::vec3(0, 1, 0)));
                else tangent = glm::normalize(glm::cross(n, glm::vec3(1, 0, 0)));
                vertices[idx].tangent = glm::vec4(tangent, 1.0f);
            }
        }
    }
}

void LoadedGLTF::Draw(const glm::mat4 &topMatrix, std::vector<RenderObject> &ctx) {
    for (auto &n: topNodes_) {
        n->Draw(topMatrix, ctx);
    }
}

LoadedGLTF::LoadedGLTF(const std::filesystem::path &filePath, GLTFMetallicRoughness *gltfMaterial,
                       AllocatedImage *irradianceMap, AllocatedImage *radianceMap) {
    spdlog::info("Loading GLTF: {}", filePath.string());
    auto data = fastgltf::GltfDataBuffer::FromPath(filePath);
    CHECK(data);
    fastgltf::GltfDataBuffer dataBuffer = std::move(data.get());
    constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble |
                                 fastgltf::Options::LoadExternalBuffers;
    fastgltf::Asset gltf;
    fastgltf::Parser parser{};

    auto load = parser.loadGltf(dataBuffer, filePath.parent_path(), gltfOptions);
    CHECK(load);
    gltf = std::move(load.get());
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(GetPhysicalDevice(), &properties);

    std::vector<uint32_t> samplerIdxes;
    for (fastgltf::Sampler &sampler: gltf.samplers) {
        auto samplerInfo = InitSamplerCreateInfo(
            GltfFilterToVkFilter(sampler.minFilter.value_or(fastgltf::Filter::Nearest)),
            GltfFilterToVkFilter(sampler.magFilter.value_or(fastgltf::Filter::Nearest)),
            ExtractMipMapMode(sampler.minFilter.value_or(fastgltf::Filter::Nearest)),
            0,
            VK_LOD_CLAMP_NONE,
            VK_SAMPLER_ADDRESS_MODE_REPEAT
        );
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

        samplers_.emplace_back(Sampler::New(samplerInfo));
        samplerIdxes.
                emplace_back(BindlessDescriptorSet::GetInstance().RegisterSampler(samplers_.back()->GetVkHandle()));
    }
    std::vector<RefCountedPtr<MeshAsset> > meshes;
    std::vector<std::shared_ptr<RenderNode> > nodes;
    std::vector<RefCountedPtr<MaterialInstance> > materials;

    std::vector<VkFormat> imageFormats(gltf.images.size(), VK_FORMAT_R8G8B8A8_UNORM);

    for (const auto &material: gltf.materials) {
        auto markImageAsSrgb = [&](const std::optional<fastgltf::TextureInfo> &texInfo) {
            if (texInfo.has_value()) {
                const auto &texture = gltf.textures[texInfo->textureIndex];
                if (texture.imageIndex.has_value()) {
                    imageFormats[texture.imageIndex.value()] = VK_FORMAT_R8G8B8A8_SRGB;
                }
            }
        };

        markImageAsSrgb(material.pbrData.baseColorTexture);
        if (material.emissiveTexture.has_value()) {
            markImageAsSrgb(material.emissiveTexture);
        }
    }

    std::vector<uint32_t> imgIdxes;
    for (int i = 0; i < gltf.images.size(); i++) {
        auto &image = gltf.images[i];
        auto img = LoadImage(gltf, image, imageFormats[i]);
        if (img) {
            images_.push_back(std::move(img));
            imgIdxes.emplace_back(BindlessDescriptorSet::GetInstance().RegisterImage(images_.back()->GetImageView()));
        } else {
            // If we reach here, we need to think about image error handling.
            CHECK(false);
        }
    }

    std::vector<GLTFMetallicRoughness::PBRMaterialData> materialData(gltf.materials.size());

    for (int i = 0; i < gltf.materials.size(); ++i) {
        auto &m = gltf.materials[i];
        materialData[i].colorFactors = glm::vec4(
            m.pbrData.baseColorFactor[0],
            m.pbrData.baseColorFactor[1],
            m.pbrData.baseColorFactor[2],
            m.pbrData.baseColorFactor[3]
        );

        materialData[i].metalRoughFactors = glm::vec4(
            m.pbrData.metallicFactor,
            m.pbrData.roughnessFactor,
            0.0f,
            0.0f
        );

        MaterialPass passType = m.alphaMode == fastgltf::AlphaMode::Blend
                                    ? MaterialPass::OPAQUE
                                    : MaterialPass::OPAQUE;

        uint32_t colorImageIndex;
        uint32_t colorSamplerIdx;

        uint32_t normalImgIdx{};
        uint32_t normalSamplerIdx{};

        uint32_t metalRoughImgIdx{};
        uint32_t metalRoughSamplerIdx{};

        uint32_t emissiveImgIdx{};
        uint32_t emissiveSamplerIdx{};

        uint32_t occlusionImgIdx{};
        uint32_t occlusionSamplerIdx{};

        // TODO: Change to upload array of images and generate a vector of indexes, so that we won't upload twice.
        auto AssignImageAndSamplerIdx = [&](uint32_t &imgIdx, uint32_t &samplerIdx, uint32_t defaultIndex,
                                            const auto &info) {
            if (info.has_value()) {
                auto imgGltfIdx = gltf.textures[info.value().textureIndex].imageIndex;
                if (imgGltfIdx.has_value()) {
                    imgIdx = imgIdxes[imgGltfIdx.value()];
                } else {
                    imgIdx = defaultIndex;
                }
                auto samplerGltfIdx = gltf.textures[info.value().textureIndex].samplerIndex;
                if (!samplerGltfIdx.has_value()) {
                    samplerIdx = DefaultSamplers::GetInstance().GetLinearIndex();
                } else {
                    samplerIdx = samplerIdxes[samplerGltfIdx.value()];
                }
            } else {
                imgIdx = defaultIndex;
                samplerIdx = DefaultSamplers::GetInstance().GetLinearIndex();
            }
        };

        AssignImageAndSamplerIdx(colorImageIndex, colorSamplerIdx, DefaultTextures::GetInstance().GetWhiteIndex(),
                                 m.pbrData.baseColorTexture);
        AssignImageAndSamplerIdx(normalImgIdx, normalSamplerIdx, DefaultTextures::GetInstance().GetFlatNormalIndex(),
                                 m.normalTexture);
        AssignImageAndSamplerIdx(metalRoughImgIdx, metalRoughSamplerIdx, DefaultTextures::GetInstance().GetGreyIndex(),
                                 m.pbrData.metallicRoughnessTexture);
        AssignImageAndSamplerIdx(emissiveImgIdx, emissiveSamplerIdx, DefaultTextures::GetInstance().GetBlackIndex(),
                                 m.emissiveTexture);
        AssignImageAndSamplerIdx(occlusionImgIdx, occlusionSamplerIdx, DefaultTextures::GetInstance().GetWhiteIndex(),
                                 m.occlusionTexture);

        materialData[i].baseColorIdx = colorImageIndex;
        materialData[i].baseColorSamplerIdx = colorSamplerIdx;
        materialData[i].normalIdx = normalImgIdx;
        materialData[i].normalSamplerIdx = normalSamplerIdx;
        materialData[i].metallicRoughIdx = metalRoughImgIdx;
        materialData[i].metallicRoughSamplerIdx = metalRoughSamplerIdx;
        materialData[i].emissiveIdx = emissiveImgIdx;
        materialData[i].emissiveSamplerIdx = emissiveSamplerIdx;
        materialData[i].occlusionIdx = occlusionImgIdx;
        materialData[i].occlusionSamplerIdx = occlusionSamplerIdx;

        materials.emplace_back(
            gltfMaterial->WriteMaterial(passType, materialData[i])
        );
    }

    // Load mesh
    spdlog::info("[fastgltf] # of meshes: {}", gltf.meshes.size());
    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;
    std::vector<GeoSurface> geoSurfaces;
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
            // Load normals.
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
            auto tangents = p.findAttribute("TANGENT");
            if (tangents != p.attributes.end()) {
                fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[tangents->accessorIndex],
                                                              [&](glm::vec4 v, size_t index) {
                                                                  vertices[initialVtx + index].tangent = v;
                                                              });
            } else {
                spdlog::warn("No tangent data detected for model, calculating...");

                CalculateTangents(vertices, indices,
                                  static_cast<uint32_t>(geoSurfaces.back().GetStartIndex()),
                                  static_cast<uint32_t>(geoSurfaces.back().GetCount()),
                                  static_cast<uint32_t>(initialVtx),
                                  static_cast<uint32_t>(positionAccessor.count));
            }
            if (p.materialIndex.has_value()) {
                geoSurfaces.back().SetMaterial(materials[p.materialIndex.value()]);
            } else {
                geoSurfaces.back().SetMaterial(materials[0]);
            }
        }
        meshes.push_back(MeshAsset::New(vertices, indices, geoSurfaces, mesh.name));
        meshes_[mesh.name.c_str()] = meshes.back();
    }

    // Load nodes.
    for (auto &node: gltf.nodes) {
        std::shared_ptr<RenderNode> renderNode;

        glm::mat4 localTransform;
        std::visit(fastgltf::visitor{
                       [&](fastgltf::math::fmat4x4 matrix) {
                           memcpy(&localTransform, matrix.data(), sizeof(matrix));
                       },
                       [&](fastgltf::TRS transform) {
                           glm::vec3 tl(transform.translation[0], transform.translation[1],
                                        transform.translation[2]);
                           glm::quat rot(transform.rotation[3], transform.rotation[0], transform.rotation[1],
                                         transform.rotation[2]);
                           glm::vec3 sc(transform.scale[0], transform.scale[1], transform.scale[2]);

                           glm::mat4 tm = glm::translate(glm::mat4(1.f), tl);
                           glm::mat4 rm = glm::mat4_cast(rot);
                           glm::mat4 sm = glm::scale(glm::mat4(1.f), sc);

                           localTransform = tm * rm * sm;
                       }
                   },
                   node.transform);
        if (node.meshIndex.has_value()) {
            renderNode = std::make_shared<MeshNode>(localTransform, glm::mat4(1.0f), meshes[*node.meshIndex]);
        } else {
            renderNode = std::make_shared<RenderNode>(localTransform, glm::mat4(1.0f));
        }
        nodes.push_back(renderNode);
        nodes_[node.name.c_str()];
    }
    // Fill child and parent info
    for (int i = 0; i < gltf.nodes.size(); i++) {
        fastgltf::Node &node = gltf.nodes[i];
        std::shared_ptr<RenderNode> &sceneNode = nodes[i];
        for (auto &c: node.children) {
            sceneNode->AddChild(nodes[c]);
            nodes[c]->SetParent(sceneNode);
        }
    }
    for (auto &node: nodes) {
        auto parent = node->GetParent();
        if (parent == nullptr) {
            topNodes_.push_back(node);
            node->UpdateTransform(glm::mat4(1.0f));
        }
    }
}
