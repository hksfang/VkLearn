#include <spdlog/spdlog.h>
#include <structopt/app.hpp>

#include "FlyingCamera.h"
#include "OrbitCamera.h"
#include "VkRenderer.h"
#include "Window.h"
#include <chrono>
#include <filesystem>
#include <imgui.h>

struct Options {
    std::string meshFile{};
};

STRUCTOPT(Options, meshFile);

enum class CameraType {
    Orbit,
    Flying
};

int main(int argc, char *argv[]) {
    Window &window = Window::GetInstance();
    VkRenderer renderer{
        static_cast<uint32_t>(window.GetWidth()),
        static_cast<uint32_t>(window.GetHeight())
    };

    // Initialize cameras
    OrbitCamera orbitCamera{-60.0f, -30.0f, -2.5f};
    FlyingCamera flyingCamera{{0, 0, -5.0f}};
    CameraType currentCameraType = CameraType::Orbit;

    // Scan for GLB files
    std::vector<std::string> glbFiles;
    std::vector<std::string> glbFileNames;
    for (const auto &entry : std::filesystem::directory_iterator("Assets/Meshes")) {
        if (entry.path().extension() == ".glb") {
            glbFiles.push_back(entry.path().string());
            glbFileNames.push_back(entry.path().filename().string());
        }
    }
    int currentMeshIdx = -1;
    for (int i = 0; i < glbFiles.size(); ++i) {
        if (glbFiles[i].find("damaged_helmet_tangent.glb") != std::string::npos) {
            currentMeshIdx = i;
            break;
        }
    }

    auto lastTime = std::chrono::high_resolution_clock::now();

    window.EventLoop([&] {
                         renderer.DrawImGuiContent();
                         if (ImGui::Begin("Debug Window")) {
                             if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
                                 const char* cameraTypes[] = { "Orbit", "Flying" };
                                 int currentItem = static_cast<int>(currentCameraType);
                                 if (ImGui::Combo("Camera Type", &currentItem, cameraTypes, IM_ARRAYSIZE(cameraTypes))) {
                                     CameraType nextType = static_cast<CameraType>(currentItem);
                                     if (nextType != currentCameraType) {
                                         if (nextType == CameraType::Flying) {
                                             glm::vec3 pos = orbitCamera.GetPos();
                                             flyingCamera.SetPosition(pos);
                                             float dist = glm::length(pos);
                                             if (dist > 0.001f) {
                                                 glm::vec3 lookDir = -pos / dist;
                                                 float pitch = asin(std::clamp(lookDir.y, -1.0f, 1.0f));
                                                 float yaw = atan2(lookDir.x, -lookDir.z);
                                                 flyingCamera.SetRotation(pitch, yaw);
                                             }
                                         } else {
                                             glm::vec3 pos = flyingCamera.GetPos();
                                             float dist = glm::length(pos);
                                             if (dist > 0.001f) {
                                                 float pitch = asin(std::clamp(pos.y / dist, -1.0f, 1.0f));
                                                 float yaw = atan2(pos.z, pos.x);
                                                 orbitCamera.SetState(glm::degrees(yaw), glm::degrees(pitch), dist);
                                             }
                                         }
                                         currentCameraType = nextType;
                                     }
                                 }

                                 if (currentCameraType == CameraType::Flying) {
                                     ImGui::DragFloat("Speed", &flyingCamera.GetSpeed(), 0.1f, 0.1f, 100.0f);
                                     ImGui::DragFloat("Sensitivity", &flyingCamera.GetSensitivity(), 0.001f, 0.001f, 1.0f);
                                 }
                             }

                             if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
                                 if (!glbFileNames.empty()) {
                                     std::vector<const char*> names;
                                     for (const auto& name : glbFileNames) {
                                         names.push_back(name.c_str());
                                     }
                                     if (ImGui::Combo("Model", &currentMeshIdx, names.data(), static_cast<int>(names.size()))) {
                                         if (currentMeshIdx >= 0 && currentMeshIdx < glbFiles.size()) {
                                             renderer.LoadGLTF(glbFiles[currentMeshIdx]);
                                         }
                                     }
                                 } else {
                                     ImGui::Text("No .glb files found in Assets/Meshes");
                                 }
                             }
                         }
                         ImGui::End();
                     }, [&]() {
                         auto currentTime = std::chrono::high_resolution_clock::now();
                         float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
                         lastTime = currentTime;

                         if (currentCameraType == CameraType::Orbit) {
                             renderer.UpdateMainCamera(orbitCamera.GetPos(),
                                                       orbitCamera.GetProjectionMatrix(renderer.GetAspect()),
                                                       orbitCamera.GetViewMatrix()
                             );
                         } else {
                             flyingCamera.Update(deltaTime);
                             renderer.UpdateMainCamera(flyingCamera.GetPos(),
                                                       flyingCamera.GetProjectionMatrix(renderer.GetAspect()),
                                                       flyingCamera.GetViewMatrix()
                             );
                         }
                         renderer.Draw();
                     }, [&](SDL_Event *evt) {
                         if (currentCameraType == CameraType::Orbit) {
                             orbitCamera.HandleSDLInput(evt);
                         } else {
                             flyingCamera.ProcessSDLEvent(*evt);
                         }
                         renderer.HandleSDLInput(evt);
                     },
                     [&](uint32_t width, uint32_t height) {
                         renderer.Resize(width, height);
                     }
    );

    return 0;
}
