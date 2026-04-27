#include "Material.h"

#include <array>
#include <VkBootstrap.h>

#include "DefaultTextures.h"
#include "../include/BindlessDescriptorSet.h"
#include "GraphicsPipelineBuilder.h"
#include "MaterialDataBuffer.h"
#include "MeshBuffer.h"
#include "VkInit.h"

GLTFMetallicRoughness::GLTFMetallicRoughness(VkFormat colorAttachmentFormat,
                                             VkFormat depthAttachmentFormat,
                                             uint32_t radianceMapIdx,
                                             uint32_t irradianceMapIdx) : radianceMapIdx_(radianceMapIdx),
                                                                          irradianceMapIdx_(irradianceMapIdx) {
    RefCountedPtr<ShaderModule> vertexShaderModule = ShaderModule::New("Assets/Shaders/gltf_pbr.vert.spv");
    RefCountedPtr<ShaderModule> fragmentShaderModule = ShaderModule::New("Assets/Shaders/gltf_pbr.frag.spv");

    VkPushConstantRange matrixRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(DrawPushConstants),
    };

    std::array layouts = {BindlessDescriptorSet::GetInstance().GetLayout()};
    auto layoutInfo = InitPipelineLayoutCreateInfo(layouts.data(), layouts.size(), &matrixRange, 1);
    RefCountedPtr<PipelineLayout> pipelineLayout = PipelineLayout::New(layoutInfo);
    // Build two pipelines.
    GraphicsPipelineBuilder graphicsPipelineBuilder{};
    graphicsPipelineBuilder.SetVertexShader(vertexShaderModule->GetVkHandle());
    graphicsPipelineBuilder.SetFragmentShader(fragmentShaderModule->GetVkHandle());
    graphicsPipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    graphicsPipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
    graphicsPipelineBuilder.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    graphicsPipelineBuilder.SetMultisampleNone();
    graphicsPipelineBuilder.DisableBlending();
    graphicsPipelineBuilder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
    graphicsPipelineBuilder.SetDepthFormat(depthAttachmentFormat);
    graphicsPipelineBuilder.SetColorAttachmentFormat(colorAttachmentFormat);
    graphicsPipelineBuilder.SetPipelineLayout(pipelineLayout->GetVkHandle());
    opaquePipeline_ = MaterialPipeline::New(graphicsPipelineBuilder.Build(), pipelineLayout);

    graphicsPipelineBuilder.EnableBlendingAlphaBlend();
    graphicsPipelineBuilder.EnableDepthTest(false, VK_COMPARE_OP_GREATER_OR_EQUAL);
    transparentPipeline_ = MaterialPipeline::New(graphicsPipelineBuilder.Build(), pipelineLayout);
}

RefCountedPtr<MaterialInstance> GLTFMetallicRoughness::WriteMaterial(
    MaterialPass pass,
    const PBRMaterialData &materialData) {
    RefCountedPtr<MaterialPipeline> pipeline = (pass == MaterialPass::TRANSPARENT)
                                                   ? transparentPipeline_
                                                   : opaquePipeline_;
    PBRMaterialData data = materialData;
    data.irradianceIdx = irradianceMapIdx_;
    data.irradianceSamplerIdx = DefaultSamplers::GetInstance().GetLinearIndex();
    data.radianceIdx = radianceMapIdx_;
    data.radianceSamplerIdx = DefaultSamplers::GetInstance().GetLinearIndex();
    // Upload to SSBO
    uint32_t offset = MaterialDataBuffer::GetInstance().AppendMaterial(data);

    return MaterialInstance::New(pipeline, offset, pass);
}

CubemapSkybox::CubemapSkybox(VkFormat colorAttachmentFormat) {
    RefCountedPtr<ShaderModule> vertexShaderModule = ShaderModule::New("Assets/Shaders/skybox.vert.spv");
    RefCountedPtr<ShaderModule> fragmentShaderModule = ShaderModule::New("Assets/Shaders/skybox.frag.spv");

    VkPushConstantRange pcRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(DrawPushConstants),
    };

    std::array layouts{BindlessDescriptorSet::GetInstance().GetLayout()};

    auto layoutInfo = InitPipelineLayoutCreateInfo(layouts.data(), layouts.size(), &pcRange,
                                                   1);
    RefCountedPtr<PipelineLayout> pipelineLayout = PipelineLayout::New(layoutInfo);

    // Config pipeline
    GraphicsPipelineBuilder graphicsPipelineBuilder{};
    graphicsPipelineBuilder.SetPipelineLayout(pipelineLayout->GetVkHandle());
    graphicsPipelineBuilder.SetVertexShader(vertexShaderModule->GetVkHandle());
    graphicsPipelineBuilder.SetFragmentShader(fragmentShaderModule->GetVkHandle());
    graphicsPipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    graphicsPipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
    graphicsPipelineBuilder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
    graphicsPipelineBuilder.SetMultisampleNone();
    graphicsPipelineBuilder.DisableBlending();
    graphicsPipelineBuilder.DisableDepthTest();
    graphicsPipelineBuilder.SetColorAttachmentFormat(colorAttachmentFormat);
    // Just for placeholder.
    graphicsPipelineBuilder.SetDepthFormat(VK_FORMAT_D32_SFLOAT);

    RefCountedPtr<GraphicsPipeline> pipeline = graphicsPipelineBuilder.Build();

    pipeline_ = MaterialPipeline::New(pipeline, pipelineLayout);
}

RefCountedPtr<MaterialInstance> CubemapSkybox::WriteMaterial(const CubemapSkyboxData &data) {
    uint32_t offset = MaterialDataBuffer::GetInstance().AppendMaterial(data);
    return MaterialInstance::New(pipeline_, offset, MaterialPass::MAX);
}
