cmake_minimum_required(VERSION 4.1)
project(VkLearn
        VERSION 1.0
        DESCRIPTION "Learn Vulkan"
        LANGUAGES CXX
)

# Set cmake path for custom cmake files.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ShaderCompile)

# Language settings
set(CMAKE_CXX_STANDARD 20 CACHE INTERNAL "")
set(CMAKE_CXX_EXTENSIONS OFF CACHE INTERNAL "")

# Third Party stuff.
find_package(SDL3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(structopt CONFIG REQUIRED)
find_package(fastgltf CONFIG REQUIRED)
find_package(Ktx CONFIG REQUIRED)
# Vulkan related
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_subdirectory(Assets)

add_library(Include INTERFACE)
target_include_directories(Include INTERFACE Include)

add_library(ThirdParty
        ThirdParty/VMA/VmaImpl.cpp
        ThirdParty/tinyobjloader/TinyObjLoaderImpl.cpp
        ThirdParty/stb_image/std_image.cpp
)

target_link_libraries(
        ThirdParty
        PUBLIC
        GPUOpen::VulkanMemoryAllocator
        Vulkan::Vulkan
        tinyobjloader::tinyobjloader
        KTX::ktx
)

target_compile_definitions(
        ThirdParty
        PUBLIC
        GLM_ENABLE_EXPERIMENTAL
)

add_library(Renderer
        Source/VkRenderer.cpp
        Source/Window.cpp
        Source/VkCmds.cpp
        Source/VkInit.cpp
        Source/VkContext.cpp
        Source/VkRaii.cpp
        Source/VkObjects.cpp
        Source/VkFrame.cpp
        Source/AllocatedImage.cpp
        Source/ImmediateContext.cpp
        Source/ImGuiContext.cpp
        Source/GraphicsPipelineBuilder.cpp
        Source/AllocatedBuffer.cpp
        Source/MeshBuffer.cpp
        Source/OrbitCamera.cpp
        Source/MeshAsset.cpp
        Source/DescriptorWriter.cpp
        Source/Material.cpp
        Source/Core/RefCount.cpp
        Source/RenderObject.cpp
        Source/FlyingCamera.cpp
        Source/DefaultTextures.cpp
        Source/LoadedGLTF.cpp
        Source/BindlessDescriptorSet.cpp
        Source/MaterialDataBuffer.cpp
)
target_link_libraries(Renderer PUBLIC
        Include
        ThirdParty
        SDL3::SDL3
        spdlog::spdlog
        glm::glm
        vk-bootstrap::vk-bootstrap
        Vulkan::Vulkan
        imgui::imgui
        tinyobjloader::tinyobjloader
        fastgltf::fastgltf
)

add_executable(VkLearn main.cpp)
target_link_libraries(VkLearn PRIVATE  Renderer Include structopt::structopt)
add_dependencies(VkLearn Assets)