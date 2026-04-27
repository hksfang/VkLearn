#pragma once

#include <vector>
#include <functional>

class DeletionQueue {
public:
    ~DeletionQueue() {
        Flush();
    }

    template<typename F>
    void PushFunction(F &&function) {
        deletors_.emplace_back(std::forward<F>(function));
    }

    void Flush() {
        for (auto it = deletors_.rbegin(); it != deletors_.rend(); ++it) {
            (*it)();
        }

        deletors_.clear();
    }

private:
    std::vector<std::function<void()> > deletors_;
};
