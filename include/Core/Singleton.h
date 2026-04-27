#pragma once
#include "NonMoveCopyable.h"

template<typename T>
class Singleton : NonCopyOrMovable {
public:
    static T &GetInstance() {
        static InstanceT singleton{};
        return singleton;
    }

protected:
    Singleton() = default;

    ~Singleton() = default;

private:
    struct InstanceT : T {
        InstanceT() = default;
        ~InstanceT() = default;
    };
};
