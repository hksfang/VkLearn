#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vk_enum_string_helper.h>

#define VK_CHECK(x)                                                     \
    do {                                                                \
        VkResult err = x;                                               \
        if (err) {                                                      \
            spdlog::error("Vulkan error: {}, aborting...", string_VkResult(err)); \
            std::abort();                                                    \
        }                                                               \
    } while (0)
