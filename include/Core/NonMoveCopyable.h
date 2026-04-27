#pragma once

class NonCopyable {
public:
    NonCopyable() = default;

    NonCopyable(const NonCopyable &) = delete;

    NonCopyable &operator=(const NonCopyable &) = delete;
};

// Move is deleted when copy is deleted unless explicitly stated otherwise
using NonCopyOrMovable = NonCopyable;
