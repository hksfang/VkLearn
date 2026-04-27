#pragma once
#include <array>

#include "AllocatedImage.h"
#include "Core/Singleton.h"
#include <glm/glm.hpp>


class DefaultTextures : public Singleton<DefaultTextures> {
public:
    AllocatedImage *GetBlack() {
        return black_.get();
    }

    AllocatedImage *GetGrey() {
        return grey_.get();
    }

    AllocatedImage *GetWhite() {
        return white_.get();
    }

    AllocatedImage *GetErrorCheckerboard() {
        return errorCheckerboard_.get();
    }

    AllocatedImage *GetFlatNormal() {
        return flatNormal_.get();
    }

    uint32_t GetWhiteIndex() const { return whiteIndex_; }
    uint32_t GetBlackIndex() const { return blackIndex_; }
    uint32_t GetGreyIndex() const { return greyIndex_; }
    uint32_t GetFlatNormalIndex() const { return flatNormalIndex_; }
    uint32_t GetErrorCheckerboardIndex() const { return errorCheckerboardIndex_; }

protected:
    DefaultTextures();

private:
    std::unique_ptr<AllocatedImage> white_;
    std::unique_ptr<AllocatedImage> black_;
    std::unique_ptr<AllocatedImage> grey_;
    std::unique_ptr<AllocatedImage> flatNormal_;
    std::unique_ptr<AllocatedImage> errorCheckerboard_;

    uint32_t whiteIndex_;
    uint32_t blackIndex_;
    uint32_t greyIndex_;
    uint32_t flatNormalIndex_;
    uint32_t errorCheckerboardIndex_;
};

class DefaultSamplers : public Singleton<DefaultSamplers> {
public:
    RefCountedPtr<Sampler> GetNearest() {
        return nearest_;
    }

    RefCountedPtr<Sampler> GetLinear() {
        return linear_;
    }

    RefCountedPtr<Sampler> GetCompare() {
        return compare_;
    }

    uint32_t GetNearestIndex() const { return nearestIndex_; }
    uint32_t GetLinearIndex() const { return linearIndex_; }
    uint32_t GetCompareIndex() const { return compareIndex_; }

protected:
    DefaultSamplers();

private:
    RefCountedPtr<Sampler> linear_;
    RefCountedPtr<Sampler> nearest_;
    RefCountedPtr<Sampler> compare_;

    uint32_t linearIndex_;
    uint32_t nearestIndex_;
    uint32_t compareIndex_;
};
