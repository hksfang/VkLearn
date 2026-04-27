#pragma once
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <thread>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

#include "Core/Singleton.h"

class Window : public Singleton<Window> {
public:
    SDL_Window *GetWindow() const {
        return window_;
    }

    int GetWidth() const {
        return width_;
    }

    int GetHeight() const {
        return height_;
    }

    template<typename ImGUIDrawFunc, typename DrawFunc, typename EventProcessFunc, typename ResizeFunc>
    void EventLoop(ImGUIDrawFunc imguiDrawFunc, DrawFunc drawFunc, EventProcessFunc processEvent,
                   ResizeFunc externalResize) {
        SDL_Event evt;
        bool quit = false;
        bool pauseRendering = false;
        while (!quit) {
            while (SDL_PollEvent(&evt)) {
                if (evt.type == SDL_EVENT_KEY_DOWN) {
                    if (evt.key.key == SDLK_ESCAPE) {
                        quit = true;
                    }
                }
                if (evt.type == SDL_EVENT_QUIT) {
                    quit = true;
                }
                if (evt.type == SDL_EVENT_WINDOW_RESIZED) {
                    // TODO: Resize window and swapchain stuff.
                }
                if (evt.type == SDL_EVENT_WINDOW_MINIMIZED) {
                    pauseRendering = true;
                }
                if (evt.type == SDL_EVENT_WINDOW_RESTORED) {
                    pauseRendering = false;
                }
                ImGui_ImplSDL3_ProcessEvent(&evt);
                processEvent(&evt);
            }
            if (pauseRendering) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            if (CheckResize()) {
                externalResize(width_, height_);
            }
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
            imguiDrawFunc();
            ImGui::Render();
            drawFunc();
        }
    }

protected:
    Window(int initW = 800, int initH = 600);

    ~Window();

private:
    bool CheckResize();

    int width_;
    int height_;
    SDL_Window *window_;
};
