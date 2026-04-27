#pragma once

#include <glm/glm.hpp>

#include "DescriptorWriter.h"
#include "VkObjects.h"
#include "Core/RefCount.h"

enum class MaterialPass {
    OPAQUE,
    TRANSPARENT,
    MAX
};

class MaterialPipeline : public RefCounted<MaterialPipeline> {
public:
    VkPipelineLayout GetVkPipelineLayout() const {
        return layout_->GetVkHandle();
    }

    VkPipeline GetPipeline() const {
        return pipeline_->GetVkHandle();
    }

protected:
    MaterialPipeline(RefCountedPtr<GraphicsPipeline> pipeline,
                     RefCountedPtr<PipelineLayout> layout) : pipeline_(pipeline),
                                                             layout_(layout) {
    }

private:
    RefCountedPtr<GraphicsPipeline> pipeline_;
    RefCountedPtr<PipelineLayout> layout_;
};

class MaterialInstance : public RefCounted<MaterialInstance> {
public:
    RefCountedPtr<MaterialPipeline> GetMaterialPipeline() const {
        return pipeline_;
    }


    uint32_t GetDataBufferOffset() const {
        return dataBufferOffset_;
    }

    MaterialPass GetPassType() const {
        return passType_;
    }

protected:
    MaterialInstance(RefCountedPtr<MaterialPipeline> pipeline, uint32_t dataBufferOffset,
                     MaterialPass pass) : pipeline_(pipeline),
                                          dataBufferOffset_(dataBufferOffset),
                                          passType_(pass) {
    }

private:
    RefCountedPtr<MaterialPipeline> pipeline_;
    uint32_t dataBufferOffset_;
    MaterialPass passType_;
};

class GLTFMetallicRoughness {
public:
    struct PBRMaterialData {
        glm::vec4 colorFactors;
        glm::vec4 metalRoughFactors;
        uint32_t baseColorIdx;
        uint32_t baseColorSamplerIdx;
        uint32_t normalIdx;
        uint32_t normalSamplerIdx;
        uint32_t metallicRoughIdx;
        uint32_t metallicRoughSamplerIdx;
        uint32_t radianceIdx;
        uint32_t radianceSamplerIdx;
        uint32_t irradianceIdx;
        uint32_t irradianceSamplerIdx;
        uint32_t emissiveIdx;
        uint32_t emissiveSamplerIdx;
        uint32_t occlusionIdx;
        uint32_t occlusionSamplerIdx;
    };

    GLTFMetallicRoughness(VkFormat colorAttachmentFormat,
                          VkFormat depthAttachmentFormat,
                          uint32_t radianceMapIdx,
                          uint32_t irradianceMapIdx);

    RefCountedPtr<MaterialInstance> WriteMaterial(
        MaterialPass pass,
        const PBRMaterialData &materialData);

    RefCountedPtr<MaterialPipeline> GetMaterialPipeline(MaterialPass pass) {
        return pass == MaterialPass::OPAQUE ? opaquePipeline_ : transparentPipeline_;
    }

private:
    RefCountedPtr<MaterialPipeline> opaquePipeline_{};
    RefCountedPtr<MaterialPipeline> transparentPipeline_{};
    RefCountedPtr<DescriptorSetLayout> layout_{};
    uint32_t radianceMapIdx_;
    uint32_t irradianceMapIdx_;
};

class CubemapSkybox {
public:
    struct CubemapSkyboxData {
        uint32_t cubemapIdx;
        uint32_t cubemapSamplerIdx;
    };

    CubemapSkybox(VkFormat colorAttachmentFormat);

    RefCountedPtr<MaterialInstance> WriteMaterial(const CubemapSkyboxData &data);

    VkDescriptorSetLayout GetVkDescriptorSetLayout() const {
        return layout_->GetVkHandle();
    }


    RefCountedPtr<MaterialPipeline> GetMaterialPipeline() {
        return pipeline_;
    }

private:
    DescriptorWriter writer_;
    RefCountedPtr<MaterialPipeline> pipeline_;
    RefCountedPtr<DescriptorSetLayout> layout_;
};
