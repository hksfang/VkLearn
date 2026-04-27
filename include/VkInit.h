#pragma once

#include <span>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>


VkCommandPoolCreateInfo InitCommandPoolCreateInfo(uint32_t qfIndex, VkCommandPoolCreateFlags flags = 0);

VkCommandBufferAllocateInfo InitCommandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1);

VkFenceCreateInfo InitFenceCreateInfo(VkFenceCreateFlags flags = 0);

VkSemaphoreCreateInfo InitSemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

VkCommandBufferBeginInfo InitCommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);

VkImageSubresourceRange InitImageSubresourceRange(VkImageAspectFlags aspectMask);

VkSemaphoreSubmitInfo InitSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);

VkCommandBufferSubmitInfo InitCommandBufferSubmitInfo(VkCommandBuffer cmd);

VkSubmitInfo2 InitSubmitInfo(VkCommandBufferSubmitInfo *cmdSubmitInfo, VkSemaphoreSubmitInfo *signalSemaphoreInfo,
                             VkSemaphoreSubmitInfo *waitSemaphoreInfo);

VkImageCreateInfo InitImageCreateInfo(VkFormat format,
                                      VkImageUsageFlags flags,
                                      uint32_t arrayLayers,
                                      VkImageUsageFlags usageFlags,
                                      VkExtent3D extent,
                                      uint32_t mipLevels = 1);

VkImageViewCreateInfo InitImageViewCreateInfo(
    VkFormat format,
    VkImage image,
    VkImageAspectFlags aspectFlags,
    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D,
    uint32_t arrayLayers = 1,
    uint32_t mipLevels = 1
);

VmaAllocationCreateInfo InitVmaAllocationCreateInfo(VmaMemoryUsage usageFlags, VkMemoryPropertyFlags propertyFlags,
                                                    VmaAllocationCreateFlags flags);

VkDescriptorSetLayoutCreateInfo InitDescriptorSetLayoutCreateInfo(std::span<const VkDescriptorSetLayoutBinding> binding,
                                                                  VkDescriptorSetLayoutCreateFlags flags = 0);

VkDescriptorPoolCreateInfo InitDescriptorPoolCreateInfo(std::span<const VkDescriptorPoolSize> poolSizes, int maxSets,
                                                        VkDescriptorPoolCreateFlags flags);

VkShaderModuleCreateInfo InitShaderModuleCreateInfo(std::span<const uint32_t> data);

VkPipelineLayoutCreateInfo InitPipelineLayoutCreateInfo(const VkDescriptorSetLayout *layouts, uint32_t layoutCount,
                                                        const VkPushConstantRange *pushConstantRanges = nullptr,
                                                        uint32_t pushConstantCount = 0);

VkPipelineShaderStageCreateInfo InitShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule module,
                                                          const char *name);

VkComputePipelineCreateInfo InitComputePipelineCreateInfo(VkPipelineLayout pipelineLayout,
                                                          VkPipelineShaderStageCreateInfo stageInfo);

VkRenderingAttachmentInfo InitAttachmentInfo(VkImageView view, VkClearValue *clear,
                                             VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

VkRenderingAttachmentInfo InitDepthAttachmentInfo(VkImageView view,
                                                  VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                                  VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);

VkRenderingInfo InitRenderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo *colorAttachment,
                                  VkRenderingAttachmentInfo *depthAttachment);

VkRenderingInfo InitRenderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo *colorAttachments,
                                  uint32_t colorAttachmentCount,
                                  VkRenderingAttachmentInfo *depthAttachment);

VkImageMemoryBarrier2 InitImageMemoryBarrier2(VkImage image, VkPipelineStageFlags2 srcStage, VkAccessFlags2 srcAccess,
                                              VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess,
                                              VkImageLayout oldLayout, VkImageLayout newLayout);

// Graphics Pipelines
VkPipelineVertexInputStateCreateInfo InitVertexInputStateCreateInfo();

VkPipelineInputAssemblyStateCreateInfo InitInputAssemblyStateCreateInfo();

VkPipelineTessellationStateCreateInfo InitTessellationStateCreateInfo();

VkPipelineViewportStateCreateInfo InitViewportStateCreateInfo();

VkPipelineRasterizationStateCreateInfo InitRasterizationStateCreateInfo();

VkPipelineMultisampleStateCreateInfo InitMultisampleStateCreateInfo();

VkPipelineDepthStencilStateCreateInfo InitDepthStencilStateCreateInfo();

VkPipelineColorBlendAttachmentState InitColorBlendAttachmentState();

VkPipelineDynamicStateCreateInfo InitDynamicStateCreateInfo();

VkPipelineRenderingCreateInfo InitRenderingCreateInfo();

VkPipelineColorBlendStateCreateInfo InitColorBlendStateCreateInfo(
    std::span<const VkPipelineColorBlendAttachmentState> colorBlendAttachments, VkBool32 logicOpEnable,
    VkLogicOp logicOp);

VkBufferCreateInfo InitBufferCreateInfo(size_t allocSize, VkBufferUsageFlags usage);

VkSamplerCreateInfo InitSamplerCreateInfo(
    VkFilter minFilter,
    VkFilter magFilter,
    VkSamplerMipmapMode mipmapMode,
    float minLod,
    float maxLod,
    VkSamplerAddressMode addressMode);
