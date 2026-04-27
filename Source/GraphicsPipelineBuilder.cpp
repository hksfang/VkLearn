#include "GraphicsPipelineBuilder.h"

#include "VkInit.h"

void GraphicsPipelineBuilder::Clear() {
    inputAssembly_ = InitInputAssemblyStateCreateInfo();
    rasterizer_ = InitRasterizationStateCreateInfo();
    colorBlendAttachment_ = InitColorBlendAttachmentState();
    multisampling_ = InitMultisampleStateCreateInfo();
    pipelineLayout_ = {};
    depthStencil_ = InitDepthStencilStateCreateInfo();
    renderInfo_ = InitRenderingCreateInfo();
    shaderStages_.clear();
    colorAttachmentFormats_.clear();
}

RefCountedPtr<GraphicsPipeline> GraphicsPipelineBuilder::Build() {
    RefCountedPtr<GraphicsPipeline> result;
    auto viewportStateCreateInfo = InitViewportStateCreateInfo();
    
    std::vector<VkPipelineColorBlendAttachmentState> colorAttachments;
    if (colorAttachmentFormats_.empty()) {
        colorAttachments.push_back(colorBlendAttachment_);
    } else {
        for (size_t i = 0; i < colorAttachmentFormats_.size(); ++i) {
            colorAttachments.push_back(colorBlendAttachment_);
        }
    }

    auto colorBlending = InitColorBlendStateCreateInfo(
        colorAttachments,
        VK_FALSE,
        VK_LOGIC_OP_COPY);
    auto vertexCreateInfo = InitVertexInputStateCreateInfo();

    // Build actual pipeline
    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderInfo_,
        .stageCount = static_cast<uint32_t>(shaderStages_.size()),
        .pStages = shaderStages_.data(),
        .pVertexInputState = &vertexCreateInfo,
        .pInputAssemblyState = &inputAssembly_,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizer_,
        .pMultisampleState = &multisampling_,
        .pDepthStencilState = &depthStencil_,
        .pColorBlendState = &colorBlending,
        .layout = pipelineLayout_,
    };

    if (tessellationState_.patchControlPoints != 0) {
        graphicsPipelineCreateInfo.pTessellationState = &tessellationState_;
    }

    // Set dynamic info.
    std::vector states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .dynamicStateCount = static_cast<uint32_t>(states.size()),
        .pDynamicStates = states.data(),
    };
    graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;

    result = GraphicsPipeline::New(graphicsPipelineCreateInfo);
    return result;
}

void GraphicsPipelineBuilder::SetVertexShader(VkShaderModule vertexShader) {
    shaderStages_.emplace_back(
        InitShaderStageCreateInfo(
            VK_SHADER_STAGE_VERTEX_BIT,
            vertexShader,
            "main"
        )
    );
}

void GraphicsPipelineBuilder::SetTessellationControlShader(VkShaderModule TCS) {
    shaderStages_.emplace_back(
        InitShaderStageCreateInfo(
            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
            TCS,
            "main"
        )
    );
}

void GraphicsPipelineBuilder::SetTessellationEvaluationShader(VkShaderModule TES) {
    shaderStages_.emplace_back(
        InitShaderStageCreateInfo(
            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
            TES,
            "main"
        )
    );
}

void GraphicsPipelineBuilder::SetTessellationControlPointCount(int count) {
    tessellationState_.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellationState_.pNext = nullptr;
    tessellationState_.patchControlPoints = count;
}

void GraphicsPipelineBuilder::SetGeometryShader(VkShaderModule geometryShader) {
    shaderStages_.emplace_back(
        InitShaderStageCreateInfo(
            VK_SHADER_STAGE_GEOMETRY_BIT,
            geometryShader,
            "main"
        )
    );
}

void GraphicsPipelineBuilder::SetFragmentShader(VkShaderModule fragmentShader) {
    shaderStages_.emplace_back(
        InitShaderStageCreateInfo(
            VK_SHADER_STAGE_FRAGMENT_BIT,
            fragmentShader,
            "main"
        )
    );
}

void GraphicsPipelineBuilder::SetPipelineLayout(VkPipelineLayout pipelineLayout) {
    pipelineLayout_ = pipelineLayout;
}

void GraphicsPipelineBuilder::SetInputTopology(VkPrimitiveTopology topology) {
    inputAssembly_.topology = topology;
    inputAssembly_.primitiveRestartEnable = VK_FALSE;
}

void GraphicsPipelineBuilder::SetPolygonMode(VkPolygonMode polygonMode) {
    rasterizer_.polygonMode = polygonMode;
    rasterizer_.lineWidth = 1.0f;
}

void GraphicsPipelineBuilder::SetCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace) {
    rasterizer_.cullMode = cullMode;
    rasterizer_.frontFace = frontFace;
}

void GraphicsPipelineBuilder::SetMultisampleNone() {
    multisampling_.sampleShadingEnable = VK_FALSE;
    multisampling_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling_.minSampleShading = 1.0f;
    multisampling_.pSampleMask = VK_NULL_HANDLE;
    multisampling_.alphaToCoverageEnable = VK_FALSE;
    multisampling_.alphaToOneEnable = VK_FALSE;
}

void GraphicsPipelineBuilder::DisableBlending() {
    colorBlendAttachment_.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;

    colorBlendAttachment_.blendEnable = VK_FALSE;
}

void GraphicsPipelineBuilder::EnableBlendingAdditive() {
    colorBlendAttachment_.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment_.blendEnable = VK_TRUE;
    colorBlendAttachment_.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment_.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment_.colorBlendOp = VK_BLEND_OP_ADD;
    // TODO: This doesn't seem right. The additive blending should not touch the alpha channel on framebuffer.
    colorBlendAttachment_.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment_.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment_.alphaBlendOp = VK_BLEND_OP_ADD;
}

void GraphicsPipelineBuilder::EnableBlendingAlphaBlend() {
    colorBlendAttachment_.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment_.blendEnable = VK_TRUE;
    colorBlendAttachment_.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment_.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment_.colorBlendOp = VK_BLEND_OP_ADD;

    colorBlendAttachment_.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment_.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment_.alphaBlendOp = VK_BLEND_OP_ADD;
}

void GraphicsPipelineBuilder::SetColorAttachmentFormats(const std::vector<VkFormat> &colorAttachmentFormats) {
    colorAttachmentFormats_ = colorAttachmentFormats;
    renderInfo_.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentFormats_.size());
    renderInfo_.pColorAttachmentFormats = colorAttachmentFormats_.data();
}

void GraphicsPipelineBuilder::SetColorAttachmentFormat(VkFormat colorAttachmentFormat) {
    colorAttachmentFormats_ = {colorAttachmentFormat};
    renderInfo_.colorAttachmentCount = 1;
    renderInfo_.pColorAttachmentFormats = colorAttachmentFormats_.data();
}

void GraphicsPipelineBuilder::SetDepthFormat(VkFormat format) {
    renderInfo_.depthAttachmentFormat = format;
}

void GraphicsPipelineBuilder::SetDepthBias(float constant, float slope) {
    rasterizer_.depthBiasEnable = VK_TRUE;
    // rasterizer_.depthBiasConstantFactor = constant;
    // rasterizer_.depthBiasSlopeFactor = slope;
}

void GraphicsPipelineBuilder::EnableDepthTest(bool enableDepthWrite, VkCompareOp compareOp) {
    depthStencil_.depthTestEnable = VK_TRUE;
    depthStencil_.depthWriteEnable = enableDepthWrite;
    depthStencil_.depthCompareOp = compareOp;
    depthStencil_.depthBoundsTestEnable = VK_FALSE;
    depthStencil_.stencilTestEnable = VK_FALSE;
    depthStencil_.front = {};
    depthStencil_.back = {};
    depthStencil_.minDepthBounds = 0.0f;
    depthStencil_.maxDepthBounds = 1.0f;
}

void GraphicsPipelineBuilder::DisableDepthTest() {
    depthStencil_.depthTestEnable = VK_FALSE;
    depthStencil_.depthWriteEnable = VK_FALSE;
    depthStencil_.depthCompareOp = VK_COMPARE_OP_NEVER;
    depthStencil_.depthBoundsTestEnable = VK_FALSE;
    depthStencil_.stencilTestEnable = VK_FALSE;
    depthStencil_.front = {};
    depthStencil_.back = {};
    depthStencil_.minDepthBounds = 0.0f;
    depthStencil_.maxDepthBounds = 1.0f;
}
