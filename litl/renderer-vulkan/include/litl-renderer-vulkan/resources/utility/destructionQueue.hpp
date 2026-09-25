#ifndef LITL_RENDERER_VULKAN_DESTRUCTION_QUEUE_H__
#define LITL_RENDERER_VULKAN_DESTRUCTION_QUEUE_H__

#include <vector>

#include "litl-renderer-vulkan/common.hpp"

namespace litl::vulkan
{
    class DestructionQueue final
    {
        enum class DestructionResourceType : uint32_t
        {
            Pipeline     = 0u,
            ShaderModule = 1u,
            Buffer       = 2u,
            SampledImage = 3u,
            // ... add others as needed ...
        };

        struct DestructionBuffer
        {
            VkBuffer vkBuffer{ VK_NULL_HANDLE };
            VmaAllocation vmaAllocation{ VK_NULL_HANDLE };
        };

        struct DestructionImage
        {
            VkImage vkImage{ VK_NULL_HANDLE };
            VkImageView vkImageView{ VK_NULL_HANDLE };
        };

        struct DestructionItem
        {
            DestructionResourceType type;
            uint32_t frames{ 2u };

            union
            {
                VkPipeline vkPipeline;
                VkShaderModule vkShaderModule;
                DestructionBuffer destructionBuffer;
                DestructionImage destructionImage;
            };
        };
    public:

        DestructionQueue() = default;
        ~DestructionQueue() = default;

        DestructionQueue(DestructionQueue const&) = delete;
        DestructionQueue& operator=(DestructionQueue const&) = delete;

        void build(VkDevice vkDevice, VmaAllocator vmaAllocator, uint32_t frameDelay) noexcept;
        void process() noexcept;

        void enqueue(VkPipeline vkPipeline) noexcept;
        void enqueue(VkShaderModule vkShaderModule) noexcept;
        void enqueue(VkBuffer vkBuffer, VmaAllocation vmaAllocation) noexcept;
        void enqueue(VkImage vkImage, VkImageView vkImageView) noexcept;

    private:

        VkDevice m_vkDevice = VK_NULL_HANDLE;
        VmaAllocator m_vmaAllocator = VK_NULL_HANDLE;
        uint32_t m_frameDelay{ 2u };

        std::vector<DestructionItem> m_toDestroy;
        std::vector<size_t> m_toDestroyIndices;
    };
}

#endif