#ifndef LITL_RENDERER_VULKAN_DESTRUCTION_QUEUE_H__
#define LITL_RENDERER_VULKAN_DESTRUCTION_QUEUE_H__

#include <vector>

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/buffer.hpp"
#include "litl-renderer/resources/texture.hpp"

namespace litl::vulkan
{
    class RendererContext;

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
                BufferHandle bufferHandle;
                TextureResourceHandle textureHandle;
            };
        };
    public:

        DestructionQueue() = default;
        ~DestructionQueue() = default;

        DestructionQueue(DestructionQueue const&) = delete;
        DestructionQueue& operator=(DestructionQueue const&) = delete;

        void build(RendererContext& rendererContext) noexcept;
        void process() noexcept;

        void enqueue(VkPipeline vkPipeline) noexcept;
        void enqueue(VkShaderModule vkShaderModule) noexcept;
        void enqueue(BufferHandle bufferHandle) noexcept;
        void enqueue(TextureResourceHandle textureHandle) noexcept;

    private:

        RendererContext* m_pContext{ nullptr };
        uint32_t m_frameDelay{ 2u };

        std::vector<DestructionItem> m_toDestroy;
        std::vector<size_t> m_toDestroyIndices;
    };
}

#endif