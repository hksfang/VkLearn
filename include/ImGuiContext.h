#pragma once
#include <SDL3/SDL_video.h>

#include "VkObjects.h"
#include "Core/RefCount.h"

class IMGUIContext {
public:
    IMGUIContext(SDL_Window *window, VkFormat swapchainFormat);

    ~IMGUIContext();

private:
    RefCountedPtr<DescriptorPool> descriptorPool_;
};
