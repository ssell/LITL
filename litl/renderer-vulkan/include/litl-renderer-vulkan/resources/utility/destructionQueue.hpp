#ifndef LITL_RENDERER_VULKAN_DESTRUCTION_QUEUE_H__
#define LITL_RENDERER_VULKAN_DESTRUCTION_QUEUE_H__

#include <queue>
#include "litl-renderer-vulkan/common.hpp"

namespace litl::vulkan
{
    class DestructionQueue final
    {
        enum class DestructionResourceType : uint32_t
        {
            Pipeline     = 0u,
            ShaderModule = 1u,
            Buffer       = 2u
            // ... add others as needed ...
        };

        struct DestructionBuffer
        {
            VkBuffer vkBuffer;
            VmaAllocation vmaAllocation;
        };

        struct DestructionItem
        {
            DestructionResourceType type;

            union
            {
                VkPipeline vkPipeline;
                VkShaderModule vkShaderModule;
                DestructionBuffer destructionBuffer;
            };
        };
    public:

        DestructionQueue() = default;
        ~DestructionQueue() = default;

        DestructionQueue(DestructionQueue const&) = delete;
        DestructionQueue& operator=(DestructionQueue const&) = delete;

        void build(VkDevice vkDevice, VmaAllocator vmaAllocator) noexcept;
        void process() noexcept;
        void enqueue(VkPipeline vkPipeline) noexcept;
        void enqueue(VkShaderModule vkShaderModule) noexcept;
        void enqueue(VkBuffer vkBuffer, VmaAllocation vmaAllocation) noexcept;

    private:

        VkDevice m_vkDevice = VK_NULL_HANDLE;
        VmaAllocator m_vmaAllocator = VK_NULL_HANDLE;
        std::queue<DestructionItem> m_toDestroy;
    };
}

#endif