#pragma once
#include <algorithm>
#include <type_traits>

template <typename T>
    requires std::is_trivial_v<T>
class alignas(T) MoveOnly
{
public:
    MoveOnly() = default;
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;

    MoveOnly(MoveOnly&& other) noexcept
    {
        std::swap(value_, other.value_);
    }

    MoveOnly& operator=(MoveOnly&& other) noexcept
    {
        std::swap(value_, other.value_);
        return *this;
    }

    // TODO: Eliminate reference passing for basic types.
    MoveOnly(const T& value) : value_(value)
    {
    }

    MoveOnly& operator=(const T& value)
    {
        value_ = value;
        return *this;
    }

    operator T&() noexcept { return value_; }
    operator const T&() const noexcept { return value_; }
    T* operator&() noexcept { return &value_; }
    const T* operator&() const noexcept { return &value_; }

    T operator->() const noexcept
        requires std::is_class_v<std::remove_pointer_t<T>> && std::is_pointer_v<T>
    {
        return value_;
    }

    T* operator->() const noexcept
        requires std::is_class_v<T>
    {
        return &value_;
    }

private:
    T value_{};
};
