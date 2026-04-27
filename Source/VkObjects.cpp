#include "VkObjects.h"

#include <fstream>
#include <VkBootstrap.h>

#include "VkContext.h"
#include "VkDebug.h"
#include "VkInit.h"
#include "Core/Debug.h"


CommandBuffer::CommandBuffer(CommandPool *commandPool, uint32_t count) : commandPool_(commandPool) {
    Create(InitCommandBufferAllocateInfo(
        commandPool->GetVkHandle(), count));
}

Swapchain::Swapchain(uint32_t width, uint32_t height) {
    Init(width, height);
}

Swapchain::~Swapchain() {
    DestroyCurrent();
}

uint32_t Swapchain::AcquireNextImageIdx(VkSemaphore signalSemaphore) {
    auto vkDevice = VulkanContext::GetInstance().GetDevice();
    // Request image from swap chain.
    uint32_t swapchainImageIndex;
    VK_CHECK(
        vkAcquireNextImageKHR(vkDevice, GetVkHandle(), 100000000, signalSemaphore,
            VK_NULL_HANDLE
            , &swapchainImageIndex));
    return swapchainImageIndex;
}

VkImage Swapchain::GetImage(uint32_t imageIdx) const {
    return vkSwapchainImages_[imageIdx];
}

VkImageView Swapchain::GetImageView(uint32_t imageIdx) const {
    return vkSwapchainImageViews_[imageIdx];
}

VkSemaphore Swapchain::GetSemaphoreForImageIdx(uint32_t imageIdx) {
    return vkSwapchainImageSemaphores_[imageIdx]->GetVkHandle();
}

void Swapchain::Present(uint32_t imageIdx) {
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        // NOTE: Why is it a pointer to imageSemaphore?
        .pWaitSemaphores = vkSwapchainImageSemaphores_[imageIdx]->GetVkHandleAddr(),
        .swapchainCount = 1,
        .pSwapchains = GetVkHandleAddr(),
        .pImageIndices = &imageIdx,
    };
    auto vkGraphicsQueue = VulkanContext::GetInstance().GetGraphicsQueue();
    VK_CHECK(vkQueuePresentKHR(vkGraphicsQueue, &presentInfo));
}

void Swapchain::Init(uint32_t width, uint32_t height) {
    auto physicalDevice = VulkanContext::GetInstance().GetPhysicalDevice();
    auto vkDevice = VulkanContext::GetInstance().GetDevice();
    auto surface = VulkanContext::GetInstance().GetSurface();
    vkb::SwapchainBuilder swapchainBuilder{physicalDevice, vkDevice, surface};
    vkSwapchainFormat_ = VK_FORMAT_R8G8B8A8_SRGB;
    vkb::Swapchain vkbSwapchain = swapchainBuilder.set_desired_format(VkSurfaceFormatKHR{
                .format = vkSwapchainFormat_,
                .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
            }).set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(width, height)
            .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT).build().
            value();
    vkSwapchainExtent_ = vkbSwapchain.extent;
    handle_ = vkbSwapchain.swapchain;
    vkSwapchainImages_ = vkbSwapchain.get_images().value();
    vkSwapchainImageViews_ = vkbSwapchain.get_image_views().value();

    for (int i = 0; i < vkSwapchainImages_.size(); ++i) {
        vkSwapchainImageSemaphores_.emplace_back(Semaphore::New(InitSemaphoreCreateInfo()));
    }
}

void Swapchain::DestroyCurrent() {
    // Destory swapchain image views.
    for (auto &vkImageView: vkSwapchainImageViews_) {
        vkDestroyImageView(GetDevice(), vkImageView, nullptr);
    }
}

VkDescriptorSet DescriptorPool::Allocate(VkDescriptorSetLayout layout) {
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = GetVkHandle(),
        .descriptorSetCount = 1,
        .pSetLayouts = &layout,
    };
    VkDescriptorSet descriptorSet;
    VK_CHECK(vkAllocateDescriptorSets(GetDevice(), &descriptorSetAllocateInfo, &descriptorSet));
    return descriptorSet;
}

ShaderModule::ShaderModule(const std::filesystem::path &filePath) {
    if (!std::filesystem::exists(filePath)) {
        spdlog::error("Shader file does not exist: {}", filePath.string());
    }
    CHECK(std::filesystem::exists(filePath));
    auto fileSize = std::filesystem::file_size(filePath);
    if (fileSize % sizeof(uint32_t) != 0) {
        spdlog::error("Shader file size is not a multiple of 4: {}, size: {}", filePath.string(), fileSize);
    }
    CHECK(fileSize % sizeof(uint32_t) == 0);

    // Write to data buffer.
    std::vector<uint32_t> data(fileSize / sizeof(uint32_t));
    std::ifstream ifs(filePath, std::ios::binary);
    if (!ifs.is_open()) {
        spdlog::error("Failed to open shader file: {}", filePath.string());
    }
    CHECK(ifs.is_open());
    ifs.read(reinterpret_cast<char *>(data.data()), fileSize);

    Create(InitShaderModuleCreateInfo(data));
}
