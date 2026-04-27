#pragma once
#include "Core/MoveOnly.h"
#include <vulkan/vulkan.h>

namespace raii {
    template<typename RAIIType, typename VkType, typename InfoType>
    class RAIIBase {
    public:
        RAIIBase() = default;

        explicit RAIIBase(const InfoType &info) {
            static_cast<RAIIType *>(this)->Create(info);
        }

        virtual ~RAIIBase() = default;

        [[nodiscard]]
        VkType GetVkHandle() const {
            return handle_;
        }

        [[nodiscard]]
        const VkType *GetVkHandleAddr() const {
            return &handle_;
        }

    protected:
        MoveOnly<VkType> handle_;
    };

    // TODO: Unify swapchain creation after dropping VKB
    class Swapchain : public RAIIBase<Swapchain, VkSwapchainKHR, VkSwapchainKHR> {
    public:
        void Create(const VkSwapchainKHR &swapchain);

        ~Swapchain() override;
    };

#define DECL_RAII(Type, VkType, InfoType) \
class Type : public RAIIBase<Type, VkType, InfoType> {  \
public: \
~Type();                                            \
void Create(const InfoType& info);                  \
};

    DECL_RAII(CommandBuffer, VkCommandBuffer, VkCommandBufferAllocateInfo);

    DECL_RAII(ComputePipeline, VkPipeline, VkComputePipelineCreateInfo);

    DECL_RAII(GraphicsPipeline, VkPipeline, VkGraphicsPipelineCreateInfo);

#define DECL_SIMPLE_RAII(Type) DECL_RAII(Type, Vk##Type, Vk##Type##CreateInfo)

    DECL_SIMPLE_RAII(CommandPool);

    DECL_SIMPLE_RAII(Fence);

    DECL_SIMPLE_RAII(Image);

    DECL_SIMPLE_RAII(ImageView);

    DECL_SIMPLE_RAII(Semaphore);

    DECL_SIMPLE_RAII(DescriptorSetLayout);

    DECL_SIMPLE_RAII(DescriptorPool);

    DECL_SIMPLE_RAII(ShaderModule);

    DECL_SIMPLE_RAII(PipelineLayout);

    DECL_SIMPLE_RAII(Buffer);

    DECL_SIMPLE_RAII(Sampler);

#undef DECL_SIMPLE_RAII
#undef DECL_RAII
}
