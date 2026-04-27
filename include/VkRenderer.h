#pragma once

#include <array>
#include <VkBootstrap.h>
#include <SDL3/SDL_events.h>

#include "AllocatedImage.h"
#include "ImGuiContext.h"
#include "LoadedGLTF.h"
#include "MeshAsset.h"
#include "Scene.h"
#include "VkFrame.h"
#include "RenderObject.h"
#include <chrono>

class VkRenderer {
public:
    VkRenderer(uint32_t windowWidth, uint32_t windowHeight);

    ~VkRenderer();

    void DrawImGuiContent();

    void Draw();

    void Resize(uint32_t width, uint32_t height);

    void UpdateMainCamera(const glm::vec3 &cameraPos, const glm::mat4 &projection, const glm::mat4 &view) {
        sceneData_.proj = projection;
        sceneData_.view = view;
        sceneData_.camPosition = glm::vec4(cameraPos, 1.0f);
        sceneData_.viewProj = projection * view;
    }

    [[nodiscard]] float GetAspect() const {
        return static_cast<float>(drawExtent_.width) / static_cast<float>(drawExtent_.height);
    };

    void HandleSDLInput(SDL_Event *evt);

    void LoadGLTF(const std::string &path);

private:
    void DrawBackground(VkCommandBuffer cmd, VkImageView drawImageView, VkImageView depthImageView,
                        VkExtent2D drawExtent);

    void DrawMetallicRoughness(VkCommandBuffer cmd, VkImageView targetImageView, VkImage targetImage);

    void DrawImGui(VkCommandBuffer cmd, VkImageView targetImageView);

    void InitTextures();

    void InitMeshData();

    void InitLightData();

    void InitMaterialPipelines();

    void InitTestMaterials();

    void UpdateScene();


    // Frame
    static constexpr int kFrameOverlap = 2;

    VkFrame &CurrentFrame() {
        return vkFrames_[frameCtr_ % kFrameOverlap];
    }

    std::array<VkFrame, kFrameOverlap> vkFrames_;
    // Swapchain
    RefCountedPtr<Swapchain> swapchain_;

    // Draw resources
    std::unique_ptr<AllocatedImage> drawImage_;
    std::unique_ptr<AllocatedImage> pbrImage_;
    std::unique_ptr<AllocatedImage> depthImage_;
    VkExtent2D drawExtent_;

    // TODO: Deferred
    std::unique_ptr<AllocatedImage> gBufferPosition_;
    std::unique_ptr<AllocatedImage> gBufferNormal_;
    std::unique_ptr<AllocatedImage> gBufferUV_;

    // ImGUI Context
    IMGUIContext imguiContext_;

    // Scene & MVP data.
    SceneData sceneData_;
    InvData invData_;

    // Material Pipelines
    std::unique_ptr<GLTFMetallicRoughness> pbrMaterial_;

    // Render Objects
    std::vector<RenderObject> pbrRenderObjects_;
    std::vector<RenderObject> transparentObjects_;
    std::unordered_map<std::string, RefCountedPtr<LoadedGLTF> > loadedScenes_;

    // Skybox
    std::unique_ptr<CubemapSkybox> cubemapSkybox_;
    RefCountedPtr<MaterialInstance> cubemapMaterialInstance_{nullptr};
    std::unique_ptr<AllocatedImage> cubemapImage_;
    uint32_t cubemapImageIdx_;
    std::unique_ptr<AllocatedImage> radianceImage_;
    uint32_t radianceImageIdx_;
    std::unique_ptr<AllocatedImage> irradianceImage_;
    uint32_t irradianceImageIdx_;

    // Settings
    float sunlightIntensity_{1.0f};

    // Statistics
    std::chrono::steady_clock::time_point lastTime_;
    float frameTime_{0.0f};
    float lastFrameTime_{0.0f};
    float lastSubmissionTime_{0.0f};
    int frameCount_{0};
    std::vector<float> frameTimeHistory_;

    // Other
    int frameCtr_{0};
};
