#include "Window.h"

#include <SDL3/SDL_init.h>

#include "SDLDebug.h"

Window::Window(int initW, int initH) : width_(initW), height_(initH) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
    window_ = SDL_CreateWindow("Vulkan Renderer",
                               width_,
                               height_,
                               flags
    );
    SDL_CHECK(window_);
}

Window::~Window() {
    SDL_DestroyWindow(window_);
}

bool Window::CheckResize() {
    int newW, newH;
    SDL_GetWindowSizeInPixels(window_, &newW, &newH);
    if (newW != width_ || newH != height_) {
        width_ = newW;
        height_ = newH;
        return true;
    }
    return false;
}
