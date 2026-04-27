#include <VkInit.h>

#include "VkContext.h"

VkCommandPoolCreateInfo InitCommandPoolCreateInfo(uint32_t qfIndex, VkCommandPoolCreateFlags flags) {
    return {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags,
        .queueFamilyIndex = qfIndex,
    };
}

VkCommandBufferAllocateInfo InitCommandBufferAllocateInfo(VkCommandPool pool, uint32_t count) {
    return {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = count
    };
}

VkFenceCreateInfo InitFenceCreateInfo(VkFenceCreateFlags flags) {
    return {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags,
    };
}

VkSemaphoreCreateInfo InitSemaphoreCreateInfo(VkSemaphoreCreateFlags flags) {
    return {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags,
    };
}

VkCommandBufferBeginInfo InitCommandBufferBeginInfo(VkCommandBufferUsageFlags flags) {
    return {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = flags,
        .pInheritanceInfo = nullptr
    };
}

VkImageSubresourceRange InitImageSubresourceRange(VkImageAspectFlags aspectMask) {
    return {
        .aspectMask = aspectMask,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS
    };
}

VkSemaphoreSubmitInfo InitSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore) {
    return {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = semaphore,
        .value = 1,
        .stageMask = stageMask,
        .deviceIndex = 0
    };
}

VkCommandBufferSubmitInfo InitCommandBufferSubmitInfo(VkCommandBuffer cmd) {
    return {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .pNext = nullptr,
        .commandBuffer = cmd,
        .deviceMask = 0
    };
}

VkSubmitInfo2 InitSubmitInfo(VkCommandBufferSubmitInfo *cmdSubmitInfo, VkSemaphoreSubmitInfo *signalSemaphoreInfo,
                             VkSemaphoreSubmitInfo *waitSemaphoreInfo) {
    return {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .pNext = nullptr,
        .waitSemaphoreInfoCount = waitSemaphoreInfo ? 1u : 0u,
        .pWaitSemaphoreInfos = waitSemaphoreInfo,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = cmdSubmitInfo,
        .signalSemaphoreInfoCount = signalSemaphoreInfo ? 1u : 0u,
        .pSignalSemaphoreInfos = signalSemaphoreInfo
    };
}


VmaAllocationCreateInfo InitVmaAllocationCreateInfo(VmaMemoryUsage usageFlags, VkMemoryPropertyFlags propertyFlags,
                                                    VmaAllocationCreateFlags flags) {
    return {
        .flags = flags,
        .usage = usageFlags,
        .requiredFlags = propertyFlags
    };
}

VkDescriptorSetLayoutCreateInfo InitDescriptorSetLayoutCreateInfo(std::span<const VkDescriptorSetLayoutBinding> binding,
                                                                  VkDescriptorSetLayoutCreateFlags flags) {
    return {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags,
        .bindingCount = static_cast<uint32_t>(binding.size()),
        .pBindings = binding.data()
    };
}

VkDescriptorPoolCreateInfo InitDescriptorPoolCreateInfo(std::span<const VkDescriptorPoolSize> poolSizes, int maxSets,
                                                        VkDescriptorPoolCreateFlags flags) {
    return {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags,
        .maxSets = static_cast<uint32_t>(maxSets),
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data()
    };
}

VkShaderModuleCreateInfo InitShaderModuleCreateInfo(std::span<const uint32_t> data) {
    return {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = data.size() * sizeof(uint32_t),
        .pCode = data.data()
    };
}

VkPipelineLayoutCreateInfo InitPipelineLayoutCreateInfo(const VkDescriptorSetLayout *layouts, uint32_t count,
                                                        const VkPushConstantRange *pushConstantRanges,
                                                        uint32_t pushConstantCount) {
    return {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = count,
        .pSetLayouts = layouts,
        .pushConstantRangeCount = pushConstantCount,
        .pPushConstantRanges = pushConstantRanges,
    };
}

VkPipelineShaderStageCreateInfo InitShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule module,
                                                          const char *name) {
    return {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .stage = stage,
        .module = module,
        .pName = name,
    };
}

VkComputePipelineCreateInfo InitComputePipelineCreateInfo(VkPipelineLayout pipelineLayout,
                                                          VkPipelineShaderStageCreateInfo stageInfo) {
    return {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .stage = stageInfo,
        .layout = pipelineLayout,
    };
}

VkRenderingAttachmentInfo InitAttachmentInfo(VkImageView view, VkClearValue *clear, VkImageLayout layout) {
    VkRenderingAttachmentInfo attachmentInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = view,
        .imageLayout = layout,
        .loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    };
    if (clear) {
        attachmentInfo.clearValue = *clear;
    }
    return attachmentInfo;
}

VkRenderingAttachmentInfo InitDepthAttachmentInfo(VkImageView view, VkImageLayout layout, VkAttachmentLoadOp loadOp) {
    return {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = view,
        .imageLayout = layout,
        .loadOp = loadOp,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {
            .depthStencil = {
                .depth = 0.0f
            }
        }
    };
}

VkRenderingInfo InitRenderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo *colorAttachment,
                                  VkRenderingAttachmentInfo *depthAttachment) {
    VkRenderingInfo info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext = nullptr,
        .renderArea = VkRect2D{
            .offset = {
                .x = 0,
                .y = 0
            },
            .extent = renderExtent,
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = colorAttachment,
        .pDepthAttachment = depthAttachment,
        .pStencilAttachment = nullptr,
    };
    if (colorAttachment == nullptr) {
        info.colorAttachmentCount = 0;
    }
    return info;
}

VkRenderingInfo InitRenderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo *colorAttachments,
                                  uint32_t colorAttachmentCount,
                                  VkRenderingAttachmentInfo *depthAttachment) {
    VkRenderingInfo info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext = nullptr,
        .renderArea = VkRect2D{
            .offset = {
                .x = 0,
                .y = 0
            },
            .extent = renderExtent,
        },
        .layerCount = 1,
        .colorAttachmentCount = colorAttachmentCount,
        .pColorAttachments = colorAttachments,
        .pDepthAttachment = depthAttachment,
        .pStencilAttachment = nullptr,
    };
    return info;
}

VkImageMemoryBarrier2 InitImageMemoryBarrier2(VkImage image, VkPipelineStageFlags2 srcStage, VkAccessFlags2 srcAccess,
                                              VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess,
                                              VkImageLayout oldLayout, VkImageLayout newLayout) {
    bool isDepth = newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                   newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                   newLayout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL ||
                   newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
                   oldLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                   oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                   oldLayout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL ||
                   oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkImageMemoryBarrier2 imageBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = srcStage,
        .srcAccessMask = srcAccess,
        .dstStageMask = dstStage,
        .dstAccessMask = dstAccess,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = isDepth
                              ? (VkImageAspectFlags)VK_IMAGE_ASPECT_DEPTH_BIT
                              : (VkImageAspectFlags)VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS
        }
    };
    return imageBarrier;
}

VkPipelineVertexInputStateCreateInfo InitVertexInputStateCreateInfo() {
    return {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
    };
}

VkPipelineInputAssemblyStateCreateInfo InitInputAssemblyStateCreateInfo() {
    return {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
    };
}

VkPipelineViewportStateCreateInfo InitViewportStateCreateInfo() {
    return {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .viewportCount = 1,
        .scissorCount = 1,
    };
}

VkPipelineRasterizationStateCreateInfo InitRasterizationStateCreateInfo() {
    return {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
    };
}

VkPipelineMultisampleStateCreateInfo InitMultisampleStateCreateInfo() {
    return {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
    };
}

VkPipelineDepthStencilStateCreateInfo InitDepthStencilStateCreateInfo() {
    return {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
    };
}

VkPipelineColorBlendAttachmentState InitColorBlendAttachmentState() {
    return {};
}

VkPipelineRenderingCreateInfo InitRenderingCreateInfo() {
    return {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = nullptr,
    };
}

VkPipelineColorBlendStateCreateInfo InitColorBlendStateCreateInfo(
    std::span<const VkPipelineColorBlendAttachmentState> colorBlendAttachments,
    VkBool32 logicOpEnable,
    VkLogicOp logicOp) {
    return {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .logicOpEnable = logicOpEnable,
        .logicOp = logicOp,
        .attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size()),
        .pAttachments = colorBlendAttachments.data(),
    };
}

VkBufferCreateInfo InitBufferCreateInfo(size_t allocSize, VkBufferUsageFlags usage) {
    return {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .size = allocSize,
        .usage = usage,
    };
}

VkSamplerCreateInfo InitSamplerCreateInfo(
    VkFilter minFilter,
    VkFilter magFilter,
    VkSamplerMipmapMode mipmapMode,
    float minLod,
    float maxLod,
    VkSamplerAddressMode addressMode) {
    return {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .magFilter = magFilter,
        .minFilter = minFilter,
        .mipmapMode = mipmapMode,
        .addressModeU = addressMode,
        .addressModeV = addressMode,
        .addressModeW = addressMode,
        .minLod = minLod,
        .maxLod = maxLod,
    };
}

VkImageCreateInfo InitImageCreateInfo(
    VkFormat format,
    VkImageCreateFlags flags,
    uint32_t arrayLayers,
    VkImageUsageFlags usageFlags,
    VkExtent3D extent,
    uint32_t mipLevels) {
    return {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = extent,
        .mipLevels = mipLevels,
        .arrayLayers = arrayLayers,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usageFlags
    };
}

VkImageViewCreateInfo InitImageViewCreateInfo(
    VkFormat format,
    VkImage image,
    VkImageAspectFlags aspectFlags,
    VkImageViewType viewType,
    uint32_t layerCount,
    uint32_t mipLevels
) {
    return {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .image = image,
        .viewType = viewType,
        .format = format,
        .subresourceRange{
            .aspectMask = aspectFlags,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = layerCount,
        },
    };
}
