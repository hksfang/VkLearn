#include "ImGuiContext.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include "VkContext.h"
#include "VkDebug.h"
#include "Core/Debug.h"

IMGUIContext::IMGUIContext(SDL_Window *window, VkFormat swapchainFormat) {
    VkDescriptorPoolSize poolSizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
    };

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000,
        .poolSizeCount = std::size(poolSizes),
        .pPoolSizes = poolSizes
    };

    descriptorPool_ = DescriptorPool::New(pool_info);

    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForVulkan(window);

    // TODO: Add check_vk_result if it does not work.
    ImGui_ImplVulkan_InitInfo initInfo{
        .Instance = GetInstance(),
        .PhysicalDevice = GetPhysicalDevice(),
        .Device = GetDevice(),
        .QueueFamily = GetGraphicsQueueFamilyIndex(),
        .Queue = GetGraphicsQueue(),
        .DescriptorPool = descriptorPool_->GetVkHandle(),
        .MinImageCount = 3,
        .ImageCount = 3,
        .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        .UseDynamicRendering = true,
        .PipelineRenderingCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .pNext = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &swapchainFormat,
        },
    };
    CHECK(ImGui_ImplVulkan_Init(&initInfo));
    ImGui_ImplVulkan_CreateFontsTexture();
}

IMGUIContext::~IMGUIContext() {
    ImGui_ImplVulkan_Shutdown();
}
