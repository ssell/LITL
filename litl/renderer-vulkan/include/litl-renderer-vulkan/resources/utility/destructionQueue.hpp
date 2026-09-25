#ifndef LITL_RENDERER_VULKAN_DESTRUCTION_QUEUE_H__
#define LITL_RENDERER_VULKAN_DESTRUCTION_QUEUE_H__

#include <queue>

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/buffer.hpp"
#include "litl-renderer/resources/texture.hpp"

namespace litl::vulkan
{
    class RendererContext;

    /// <summary>
    /// A deferred queue of resources to destroy.
    /// 
    /// There is one destruction queue per frame-in-flight and so no internal tracking of "frames remaining" is required
    /// as the queue is processed at the start of the frame. So an application with 2 frames-in-flight that defers destroying
    /// a texture on Frame 5 will have the texture destroyed on Frame 7, after all per-frame command buffers are done with it.
    /// </summary>
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
        std::queue<DestructionItem> m_toDestroy;
    };
}

#endif