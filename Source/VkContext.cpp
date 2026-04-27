#include <VkContext.h>
#include <VkBootstrap.h>
#include <SDL3/SDL_vulkan.h>

#include "SDLDebug.h"
#include "VkDebug.h"
#include "VkInit.h"
#include "Window.h"

namespace {
}

void VulkanContext::DrainDevice() const {
    vkDeviceWaitIdle(device_);
}

VulkanContext::VulkanContext() {
    vkb::InstanceBuilder builder{};
    auto inst = builder.set_app_name("CS6610")
#ifdef _DEBUG
            .request_validation_layers()
#endif
            .use_default_debug_messenger().require_api_version(1, 4, 0).build();

    vkb::Instance vkInstance = inst.value();
    instance_ = vkInstance.instance;
    debugMessenger_ = vkInstance.debug_messenger;
    VkPhysicalDeviceFeatures features{
        .geometryShader = VK_TRUE,
        .tessellationShader = VK_TRUE,
        .samplerAnisotropy = VK_TRUE,
        .shaderSampledImageArrayDynamicIndexing = VK_TRUE,
        .shaderInt64 = VK_TRUE,
    };
    VkPhysicalDeviceVulkan13Features features13{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    features13.dynamicRendering = true;
    features13.synchronization2 = true;

    VkPhysicalDeviceVulkan11Features features11{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
    features11.shaderDrawParameters = true;
    features11.storagePushConstant16 = true;

    VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR derivativeFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR,
        .computeDerivativeGroupQuads = true
    };

    VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT shaderReplicatedCompositesFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_REPLICATED_COMPOSITES_FEATURES_EXT,
        .pNext = nullptr,
        .shaderReplicatedComposites = true
    };

    SDL_CHECK(SDL_Vulkan_CreateSurface(Window::GetInstance().GetWindow(), instance_, nullptr, &surface_));

    VkPhysicalDeviceVulkan12Features features12{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .runtimeDescriptorArray = true,
        .vulkanMemoryModel = true,
    };

    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;
    features12.shaderFloat16 = true;
    features12.separateDepthStencilLayouts = true;
    vkb::PhysicalDeviceSelector selector{vkInstance};
    // Create surface.

    vkb::PhysicalDevice vkbPhysicalDevice = selector
            .set_minimum_version(1, 4)
            .add_required_extension(VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME)
            .add_required_extension(VK_NV_COOPERATIVE_VECTOR_EXTENSION_NAME)
            .add_required_extension(VK_EXT_SHADER_REPLICATED_COMPOSITES_EXTENSION_NAME)
            .add_required_extension_features(derivativeFeatures)
            .add_required_extension_features(shaderReplicatedCompositesFeatures)
            .set_required_features_13(features13)
            .set_required_features_12(features12)
            .set_required_features_11(features11)
            .set_required_features(features)
            .set_surface(surface_)
            .select().value();
    spdlog::info("VKB: Selected Physical Device : {}", vkbPhysicalDevice.name);
    vkb::DeviceBuilder deviceBuilder{vkbPhysicalDevice};
    vkb::Device vkbDevice = deviceBuilder.build().value();

    device_ = vkbDevice.device;
    physicalDevice_ = vkbPhysicalDevice.physical_device;

    graphicsQueue_ = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    graphicsQueueFamilyIndex_ = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
    VmaAllocatorCreateInfo allocatorInfo{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = physicalDevice_,
        .device = device_,
        .instance = instance_,
    };

    VK_CHECK(vmaCreateAllocator(&allocatorInfo, &allocator_));
}

VulkanContext::~VulkanContext() {
    DrainDevice();

    vmaDestroyAllocator(allocator_);
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
    vkDestroyDevice(device_, nullptr);

    vkb::destroy_debug_utils_messenger(instance_, debugMessenger_, nullptr);
    vkDestroyInstance(instance_, nullptr);
}

VkInstance GetInstance() {
    return VulkanContext::GetInstance().GetVkInstance();
}

VkPhysicalDevice GetPhysicalDevice() {
    return VulkanContext::GetInstance().GetPhysicalDevice();
}

VkDevice GetDevice() {
    return VulkanContext::GetInstance().GetDevice();
}

VmaAllocator GetAllocator() {
    return VulkanContext::GetInstance().GetVMAAllocator();
}

VkQueue GetGraphicsQueue() {
    return VulkanContext::GetInstance().GetGraphicsQueue();
}

uint32_t GetGraphicsQueueFamilyIndex() {
    return VulkanContext::GetInstance().GetGraphicsQueueFamilyIndex();
}
