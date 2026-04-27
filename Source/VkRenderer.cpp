#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "VkRenderer.h"

#include <algorithm>
#include <fstream>
#include <imgui.h>
#include <stb_image.h>
#include <spdlog/spdlog.h>
#include <tiny_obj_loader.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/packing.hpp>
#include <ktx.h>
#include <cmath>

#include <glm/gtx/norm.hpp>
#include "VkContext.h"
#include "VkDebug.h"
#include "VkInit.h"
#include "VkCmds.h"
#include "Window.h"
#include "DefaultTextures.h"
#include "BindlessDescriptorSet.h"
#include "MaterialDataBuffer.h"
#include "Core/Debug.h"

namespace {
    std::unique_ptr<AllocatedImage> MakeDrawImage(uint32_t width, uint32_t height) {
        return std::make_unique<AllocatedImage>(
            0,
            1,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_STORAGE_BIT |
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT,
            VkExtent3D{
                .width = width,
                .height = height,
                .depth = 1
            },
            VK_FORMAT_R16G16B16A16_SFLOAT
        );
    }

    std::unique_ptr<AllocatedImage> MakeDepthImage(uint32_t width, uint32_t height) {
        return std::make_unique<AllocatedImage>(0, 1,
                                                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                VkExtent3D{
                                                    .width = width,
                                                    .height = height,
                                                    .depth = 1
                                                },
                                                VK_FORMAT_D32_SFLOAT
        );
    }

    std::unique_ptr<AllocatedImage> MakeRenderToTextureImage(uint32_t width, uint32_t height) {
        return std::make_unique<AllocatedImage>(
            0,
            1,
            VK_IMAGE_USAGE_SAMPLED_BIT |
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VkExtent3D{
                .width = width,
                .height = height,
                .depth = 1
            },
            VK_FORMAT_R16G16B16A16_SFLOAT
        );
    }

    std::unique_ptr<AllocatedImage> MakeGBufferImage(uint32_t width, uint32_t height, VkFormat format) {
        return std::make_unique<AllocatedImage>(
            0,
            1,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT,
            VkExtent3D{
                .width = width,
                .height = height,
                .depth = 1
            },
            format
        );
    }

    std::vector<std::byte> ReadBinaryFile(const std::filesystem::path &path) {
        auto file_size = std::filesystem::file_size(path);
        std::ifstream file(path, std::ios::binary);
        std::vector<std::byte> buffer(file_size);
        file.read(reinterpret_cast<char *>(buffer.data()), file_size);
        return buffer;
    }

    std::unique_ptr<AllocatedImage> ReadKTX(const std::filesystem::path &path) {
        ktxTexture *kTexture;
        KTX_error_code result = ktxTexture_CreateFromNamedFile(
            path.string().c_str(),
            KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
            &kTexture
        );

        if (result != 0) {
            spdlog::error("Failed to load KTX file: {}, error code: {}", path.string(), (int) result);
        }
        CHECK(result == 0);

        uint32_t width = kTexture->baseWidth;
        uint32_t height = kTexture->baseHeight;
        uint32_t mipLevels = kTexture->numLevels;
        uint32_t layers = kTexture->numLayers;

        ktx_uint8_t *pData = ktxTexture_GetData(kTexture);

        return std::make_unique<AllocatedImage>(
            pData,
            VkExtent3D{width, height, 1u},
            16,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
            6,
            VK_IMAGE_USAGE_SAMPLED_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT
        );
    }
}

VkRenderer::VkRenderer(uint32_t windowWidth, uint32_t windowHeight)
    : swapchain_(
          Swapchain::New(windowWidth, windowHeight)),
      drawImage_(MakeDrawImage(windowWidth, windowHeight)),
      pbrImage_(MakeDrawImage(windowWidth, windowHeight)),
      depthImage_(MakeDepthImage(windowWidth, windowHeight)),
      imguiContext_(
          Window::GetInstance().GetWindow(),
          swapchain_->GetFormat()),
      lastTime_(std::chrono::steady_clock::now()) {
    drawExtent_.width = drawImage_->GetExtent().width;
    drawExtent_.height = drawImage_->GetExtent().height;
    frameTimeHistory_.resize(100, 0.0f);
    gBufferPosition_ = MakeGBufferImage(windowWidth, windowHeight, VK_FORMAT_R16G16B16A16_SFLOAT);
    gBufferNormal_ = MakeGBufferImage(windowWidth, windowHeight, VK_FORMAT_R16G16B16A16_SFLOAT);
    gBufferUV_ = MakeGBufferImage(windowWidth, windowHeight, VK_FORMAT_R32G32B32A32_SFLOAT);

    InitLightData();
    InitTextures();
    InitMaterialPipelines();
    InitTestMaterials();
    InitMeshData();

    // Re-trigger Resize to ensure all pass-specific resources (like PSNR buffers/descriptor sets)
    // are correctly allocated and consistent with the initial window size.
    Resize(windowWidth, windowHeight);
}

VkRenderer::~VkRenderer() {
    VulkanContext::GetInstance().DrainDevice();
}

void VkRenderer::DrawImGuiContent() {
    if (ImGui::Begin("Debug Window")) {
        if (ImGui::CollapsingHeader("Overall Performance", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Total Frame Time: %.3f ms", lastFrameTime_);
            ImGui::Text("CPU Submission Time: %.3f ms", lastSubmissionTime_);
            ImGui::Text("FPS: %.1f", lastFrameTime_ > 0.0f ? 1000.0f / lastFrameTime_ : 0.0f);
            ImGui::PlotLines("Frame Time History", frameTimeHistory_.data(), static_cast<int>(frameTimeHistory_.size()),
                             0, nullptr, 0.0f, 33.3f, ImVec2(0, 80));
        }

        if (ImGui::CollapsingHeader("Sunlight", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat("Intensity", &sunlightIntensity_, 0.1f, 0.0f, 100.0f);
            ImGui::ColorEdit3("Sun Color", &sceneData_.sunlightColor.x);

            // Sunlight Direction
            static float phi = 45.0f;
            static float theta = 45.0f;
            static bool initialized = false;
            if (!initialized) {
                glm::vec3 dir = glm::normalize(glm::vec3(sceneData_.sunlightDirection));
                phi = glm::degrees(atan2(dir.z, dir.x));
                theta = glm::degrees(acos(dir.y));
                initialized = true;
            }
            bool dirChanged = false;
            dirChanged |= ImGui::SliderFloat("Sun Phi", &phi, 0.0f, 360.0f);
            dirChanged |= ImGui::SliderFloat("Sun Theta", &theta, 0.0f, 180.0f);

            if (dirChanged) {
                float p = glm::radians(phi);
                float t = glm::radians(theta);
                sceneData_.sunlightDirection.x = sin(t) * cos(p);
                sceneData_.sunlightDirection.y = cos(t);
                sceneData_.sunlightDirection.z = sin(t) * sin(p);
            }
        }
    }
    ImGui::End();
}

void VkRenderer::HandleSDLInput(SDL_Event *evt) {
    if (evt->key.key != SDLK_SPACE) {
        return;
    }
}

void VkRenderer::DrawBackground(VkCommandBuffer cmd, VkImageView drawImageView, VkImageView depthImageView,
                                VkExtent2D drawExtent) {
    VkClearValue clearColor;
    clearColor.color = {{0.1f, 0.1f, 0.1f, 1.0f}};
    auto colorAttachment = InitAttachmentInfo(drawImageView, &clearColor);
    auto depthAttachment = InitDepthAttachmentInfo(depthImageView);
    auto renderingInfo = InitRenderingInfo(drawExtent, &colorAttachment, &depthAttachment);
    vkCmdBeginRendering(cmd, &renderingInfo);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, cubemapSkybox_->GetMaterialPipeline()->GetPipeline());
    VkViewport viewport{
        .x = 0,
        .y = 0,
        .width = static_cast<float>(drawExtent.width),
        .height = static_cast<float>(drawExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{
        .offset = {
            .x = 0,
            .y = 0
        },
        .extent = {
            .width = drawExtent.width,
            .height = drawExtent.height,
        }
    };
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    InvData mvpData{
        .invModelMatrix = glm::mat4(1.0f),
        .invViewMatrix = glm::inverse(glm::mat4(glm::mat3(sceneData_.view))),
        .invProjMatrix = glm::inverse(sceneData_.proj),
    };
    CurrentFrame().WriteInvData(mvpData);
    DrawPushConstants pushConstants{
        .invData = CurrentFrame().GetInvAddr(),
        .vertexBuffer = 0,
        .materialDataBuffer = MaterialDataBuffer::GetInstance().GetBuffer()->GetDeviceAddress() +
                              cubemapMaterialInstance_->GetDataBufferOffset(),
        .sceneDataBuffer = CurrentFrame().GetSceneAddr(),
    };

    vkCmdPushConstants(cmd, cubemapSkybox_->GetMaterialPipeline()->GetVkPipelineLayout(),
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                       sizeof(pushConstants),
                       &pushConstants);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      cubemapMaterialInstance_->GetMaterialPipeline()->GetPipeline());
    vkCmdDraw(cmd, 3, 1, 0, 0);
    vkCmdEndRendering(cmd);
}

void VkRenderer::DrawMetallicRoughness(VkCommandBuffer cmd, VkImageView targetImageView, VkImage targetImage) {
    auto colorAttachment = InitAttachmentInfo(targetImageView, nullptr);
    auto depthAttachment = InitDepthAttachmentInfo(depthImage_->GetImageView(), VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                                   VK_ATTACHMENT_LOAD_OP_LOAD);
    auto renderingInfo = InitRenderingInfo(drawExtent_, &colorAttachment, &depthAttachment);
    vkCmdBeginRendering(cmd, &renderingInfo);
    VkViewport viewport{
        .x = 0,
        .y = 0,
        .width = static_cast<float>(drawExtent_.width),
        .height = static_cast<float>(drawExtent_.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{
        .offset = {
            .x = 0,
            .y = 0
        },
        .extent = {
            .width = drawExtent_.width,
            .height = drawExtent_.height,
        }
    };
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    // Opaque Pass
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pbrMaterial_->GetMaterialPipeline(MaterialPass::OPAQUE)->GetPipeline());
    for (auto &renderObject: pbrRenderObjects_) {
        renderObject.Draw(cmd, CurrentFrame().GetInvAddr(), CurrentFrame().GetSceneAddr());
    }

    // Transparent Pass
    if (!transparentObjects_.empty()) {
        glm::vec3 cameraPos = glm::vec3(sceneData_.camPosition);
        std::sort(transparentObjects_.begin(), transparentObjects_.end(),
                  [&](const RenderObject &a, const RenderObject &b) {
                      glm::vec3 posA = glm::vec3(a.GetModelMatrix()[3]);
                      glm::vec3 posB = glm::vec3(b.GetModelMatrix()[3]);
                      return glm::distance2(cameraPos, posA) > glm::distance2(cameraPos, posB);
                  });

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pbrMaterial_->GetMaterialPipeline(MaterialPass::TRANSPARENT)->GetPipeline());
        for (auto &renderObject: transparentObjects_) {
            renderObject.Draw(cmd, CurrentFrame().GetInvAddr(), CurrentFrame().GetSceneAddr());
        }
    }

    vkCmdEndRendering(cmd);
}

void VkRenderer::DrawImGui(VkCommandBuffer cmd, VkImageView targetImageView) {
    auto colorAttachment = InitAttachmentInfo(targetImageView, nullptr);
    auto renderInfo = InitRenderingInfo(swapchain_->GetExtent(), &colorAttachment, nullptr);
    vkCmdBeginRendering(cmd, &renderInfo);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    vkCmdEndRendering(cmd);
}

void VkRenderer::Draw() {
    // Calculate Frame Time (CPU side)
    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float, std::milli>(currentTime - lastTime_).count();
    lastTime_ = currentTime;

    // Smooth frame time for UI
    frameTime_ += deltaTime;
    frameCount_++;
    if (frameTime_ >= 200.0f) {
        // Update every 200ms
        lastFrameTime_ = frameTime_ / static_cast<float>(frameCount_);
        frameTime_ = 0.0f;
        frameCount_ = 0;

        // Update history
        for (size_t i = 0; i < frameTimeHistory_.size() - 1; ++i) {
            frameTimeHistory_[i] = frameTimeHistory_[i + 1];
        }
        frameTimeHistory_.back() = lastFrameTime_;
    }

    auto cmd = CurrentFrame().BeginFrame();
    UpdateScene();
    auto startSubmission = std::chrono::steady_clock::now();

    // 1. Get GPU Timestamps from previous time this frame index was used
    uint32_t nextImageIdx = swapchain_->AcquireNextImageIdx(CurrentFrame().GetImageAvailableSemaphore());
    VkImage swapchainImage = swapchain_->GetImage(nextImageIdx);

    // Prepare images for all passes
    ImageTransistion(cmd, pbrImage_->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    ImageTransistion(cmd, depthImage_->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
    // Bind bindless sets.
    VkDescriptorSet bindlessSet = BindlessDescriptorSet::GetInstance().GetSet();
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pbrMaterial_->GetMaterialPipeline(MaterialPass::OPAQUE)->GetVkPipelineLayout(), 0, 1,
                            &bindlessSet, 0, nullptr);

    // Pass 1: Traditional PBR
    DrawBackground(cmd, pbrImage_->GetImageView(), depthImage_->GetImageView(), drawExtent_);
    DrawMetallicRoughness(cmd, pbrImage_->GetImageView(), pbrImage_->GetImage());

    // Final composition
    ImageTransistion(cmd, pbrImage_->GetImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    ImageTransistion(cmd, swapchainImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    ImageBlit(cmd, pbrImage_->GetImage(), swapchainImage, drawExtent_, swapchain_->GetExtent());

    ImageTransistion(cmd, swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    DrawImGui(cmd, swapchain_->GetImageView(nextImageIdx));
    ImageTransistion(cmd, swapchainImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // Convert source back to Color Attachment for CalculatePSNR if needed or just future proofing
    ImageTransistion(cmd, pbrImage_->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    // End command
    VK_CHECK(vkEndCommandBuffer(cmd));

    VkSemaphore imageSemaphore = swapchain_->GetSemaphoreForImageIdx(nextImageIdx);
    auto signalInfo = InitSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, imageSemaphore);

    CurrentFrame().EndFrame(&signalInfo);
    auto endSubmission = std::chrono::steady_clock::now();
    lastSubmissionTime_ = std::chrono::duration<float, std::milli>(endSubmission - startSubmission).count();

    // Present to screen.
    swapchain_->Present(nextImageIdx);

    ++frameCtr_;
}

void VkRenderer::Resize(uint32_t width, uint32_t height) {
    VulkanContext::GetInstance().DrainDevice();
    // Rebuild swapchain
    swapchain_ = nullptr;
    swapchain_ = Swapchain::New(width, height);
    // Update draw extent
    drawExtent_.width = width;
    drawExtent_.height = height;
    // Rebuild allocated images.
    drawImage_ = MakeDrawImage(width, height);
    pbrImage_ = MakeDrawImage(width, height);
    depthImage_ = MakeDepthImage(width, height);

    gBufferPosition_ = MakeGBufferImage(width, height, VK_FORMAT_R16G16B16A16_SFLOAT);
    gBufferNormal_ = MakeGBufferImage(width, height, VK_FORMAT_R16G16B16A16_SFLOAT);
    gBufferUV_ = MakeGBufferImage(width, height, VK_FORMAT_R32G32B32A32_SFLOAT);
}

void VkRenderer::LoadGLTF(const std::string &path) {
    VulkanContext::GetInstance().DrainDevice();
    loadedScenes_.clear();
    std::filesystem::path p(path);
    std::string name = p.stem().string();
    loadedScenes_[name] = LoadedGLTF::New(path, pbrMaterial_.get(), irradianceImage_.get(), radianceImage_.get());
}

void VkRenderer::InitMeshData() {
    LoadGLTF("Assets/Meshes/damaged_helmet_tangent.glb");
}

void VkRenderer::InitLightData() {
    sceneData_.sunlightColor = glm::vec4(0.5f, 0.5f, 0.5f, 0.0f);
    sceneData_.sunlightDirection = glm::vec4(1.0f, -1.0f, 1.0f, 0.0f);
    sceneData_.ambientColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
}

void VkRenderer::InitMaterialPipelines() {
    pbrMaterial_ = std::make_unique<GLTFMetallicRoughness>(drawImage_->GetFormat(), depthImage_->GetFormat(),
                                                           radianceImageIdx_, irradianceImageIdx_);
    cubemapSkybox_ = std::make_unique<CubemapSkybox>(drawImage_->GetFormat());
}

void VkRenderer::InitTextures() {
    irradianceImage_ = ReadKTX("Assets/Skybox/irradiance_map.ktx");
    irradianceImageIdx_ = BindlessDescriptorSet::GetInstance().RegisterTextureCube(irradianceImage_->GetImageView());
    radianceImage_ = ReadKTX("Assets/Skybox/radiance_map.ktx");
    radianceImageIdx_ = BindlessDescriptorSet::GetInstance().RegisterTextureCube(radianceImage_->GetImageView());
    cubemapImage_ = ReadKTX("Assets/Skybox/skybox_cubemap.ktx");
    cubemapImageIdx_ = BindlessDescriptorSet::GetInstance().RegisterTextureCube(cubemapImage_->GetImageView());
}

void VkRenderer::InitTestMaterials() {
    CubemapSkybox::CubemapSkyboxData skyboxData{
        .cubemapIdx = cubemapImageIdx_,
        .cubemapSamplerIdx = DefaultSamplers::GetInstance().GetLinearIndex()
    };
    cubemapMaterialInstance_ = cubemapSkybox_->WriteMaterial(skyboxData);
}

void VkRenderer::UpdateScene() {
    pbrRenderObjects_.clear();
    transparentObjects_.clear();

    std::vector<RenderObject> allObjects;
    for (auto &[name, scene]: loadedScenes_) {
        scene->Draw(glm::mat4(1.0f), allObjects);
    }

    for (const auto &obj: allObjects) {
        if (obj.GetMaterialInstance()->GetPassType() == MaterialPass::TRANSPARENT) {
            transparentObjects_.push_back(obj);
        } else {
            pbrRenderObjects_.push_back(obj);
        }
    }

    sceneData_.sunlightDirection.w = sunlightIntensity_;
    CurrentFrame().WriteSceneData(sceneData_);
}
