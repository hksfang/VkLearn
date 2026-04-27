#pragma once
#include <vector>

#include "AllocatedImage.h"

class GraphicsPipelineBuilder {
public:
    GraphicsPipelineBuilder() {
        Clear();
    }

    ~GraphicsPipelineBuilder() = default;

    void Clear();

    RefCountedPtr<GraphicsPipeline> Build();


    void SetVertexShader(VkShaderModule vertexShader);

    void SetTessellationControlShader(VkShaderModule TCS);

    void SetTessellationEvaluationShader(VkShaderModule TES);

    void SetTessellationControlPointCount(int count);

    void SetGeometryShader(VkShaderModule geometryShader);

    void SetFragmentShader(VkShaderModule fragmentShader);

    void SetPipelineLayout(VkPipelineLayout pipelineLayout);

    void SetInputTopology(VkPrimitiveTopology topology);

    void SetPolygonMode(VkPolygonMode polygonMode);

    void SetCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace);

    void SetMultisampleNone();

    void DisableBlending();

    void EnableBlendingAdditive();

    void EnableBlendingAlphaBlend();

    void SetColorAttachmentFormats(const std::vector<VkFormat> &colorAttachmentFormats);

    void SetColorAttachmentFormat(VkFormat colorAttachmentFormat);

    void SetDepthFormat(VkFormat format);

    void SetDepthBias(float constant, float slope);

    void DisableDepthTest();

    void EnableDepthTest(bool enableDepthWrite, VkCompareOp compareOp);

private:
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages_;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly_;
    VkPipelineRasterizationStateCreateInfo rasterizer_;
    VkPipelineColorBlendAttachmentState colorBlendAttachment_;
    VkPipelineTessellationStateCreateInfo tessellationState_{};
    VkPipelineMultisampleStateCreateInfo multisampling_;
    VkPipelineLayout pipelineLayout_;
    VkPipelineDepthStencilStateCreateInfo depthStencil_;
    VkPipelineRenderingCreateInfo renderInfo_;
    std::vector<VkFormat> colorAttachmentFormats_;
};
