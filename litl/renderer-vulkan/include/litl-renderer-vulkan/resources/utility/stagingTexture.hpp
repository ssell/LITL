#ifndef LITL_RENDERER_VULKAN_STAGING_TEXTURE_H__
#define LITL_RENDERER_VULKAN_STAGING_TEXTURE_H__

#include <optional>
#include <span>
#include <vector>

#include "litl-core/constants.hpp"
#include "litl-renderer-vulkan/resources/buffer.hpp"
#include "litl-renderer-vulkan/resources/texture.hpp"
#include "litl-renderer-vulkan/resources/commandBuffer.hpp"

namespace litl::vulkan
{
    struct RendererContext;

    struct StagingTextureIndex
    {
        static const uint32_t FixedStagingTextureIndex = litl::Constants::uint32_null_index;

        uint64_t bufferOffset = 0ull;
        uint64_t bufferSize = 0ull;
        uint32_t bufferIndex = FixedStagingTextureIndex;
    };

    /// <summary>
    /// 
    /// </summary>
    class StagingTexture final
    {
    public:

        StagingTexture();
        ~StagingTexture();

        StagingTexture(StagingTexture const&) = delete;
        StagingTexture& operator=(StagingTexture const&) = delete;

        void build(RendererContext& context) noexcept;

        [[nodiscard]] std::optional<StagingTextureIndex> copyIntoStaging(std::span<std::byte const> source, uint64_t sourceOffset) noexcept;
        [[nodiscard]] bool copyIntoDestination(CommandBufferResource* commandBuffer, StagingTextureIndex stagingIndex, TextureResource* destination) noexcept;
        void flushBuffers(CommandBufferResource* commandBuffer);
        void freeBuffers() noexcept;

    private:

        BufferHandle createStagingBuffer(uint64_t size) noexcept;
        void flushBuffer(CommandBufferResource* commandBuffer, BufferResource* buffer) noexcept;

        RendererContext* m_pContext;
        BufferResource* m_pFixedBuffer;
        uint32_t m_fixedBufferSize;
        uint32_t m_fixedHead;

        std::vector<BufferHandle> m_overflowBuffers;
    };
}

#endif