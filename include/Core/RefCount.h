#pragma once
#include <atomic>
#include <utility>

#include "NonMoveCopyable.h"

template<typename T>
class RefCountedPtr;

template<typename T>
class RefCounted : NonCopyOrMovable {
public:
    RefCounted() = default;

    ~RefCounted() = default;

    void AddRef() {
        refCount_.fetch_add(1, std::memory_order_relaxed);
    }

    void Release() {
        auto count = refCount_.fetch_sub(1, std::memory_order_acq_rel) - 1;
        if (count == 0) {
            delete static_cast<const T *>(this);
        }
    }

    template<typename... Args>
    static RefCountedPtr<T> New(Args &&... args) {
        return new TT(std::forward<Args>(args)...);
    }

private:
    struct TT : T {
        template<typename... Args>
        explicit TT(Args &&... args) : T(std::forward<Args>(args)...) {
        }

        ~TT() = default;
    };

    std::atomic<int> refCount_{0};
};

template<typename T>
class RefCountedPtr {
public:
    RefCountedPtr() = default;

    RefCountedPtr(std::nullptr_t) {
    }

    RefCountedPtr(T *obj) : obj_(obj) {
        obj_->AddRef();
    }

    ~RefCountedPtr() {
        if (obj_) {
            obj_->Release();
        }
    }

    RefCountedPtr(const RefCountedPtr &other) {
        obj_ = other.obj_;
        if (obj_) {
            obj_->AddRef();
        }
    }

    RefCountedPtr &operator=(const RefCountedPtr &other) {
        if (obj_ == other.obj_) {
            return *this;
        }
        auto *old = std::exchange(obj_, other.obj_);
        if (old) {
            old->Release();
        }
        if (obj_) {
            obj_->AddRef();
        }
        return *this;
    }

    T *operator->() const { return obj_; }

    T *Get() const { return obj_; }

    T **Put() { return &obj_; }

private:
    T *obj_ = nullptr;
};
