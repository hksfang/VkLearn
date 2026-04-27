#pragma once

#define CHECK(x)                                                        \
    do {                                                                \
        bool res = x;                                                   \
        if (!res) {                                                     \
            spdlog::error("critical error, aborting...");               \
            std::abort();                                               \
        }                                                               \
    } while (0)
