#pragma once
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#define SDL_CHECK(x)                                                    \
    do {                                                                \
        bool res = x;                                                   \
        if (!res) {                                                     \
            spdlog::error("SDL error: {}, aborting...", SDL_GetError());\
            std::abort();                                               \
        }                                                               \
    } while (0)
