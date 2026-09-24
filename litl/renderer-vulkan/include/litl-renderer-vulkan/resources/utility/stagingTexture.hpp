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

        StagingTexture(StagingTexture const&) = delete;
        StagingTexture& operator=(StagingTexture const&) = delete;

        void build(RendererContext& context) noexcept;
        void destroy() noexcept;

        [[nodiscard]] std::optional<StagingTextureIndex> copyIntoStaging(std::span<std::byte const> source, uint64_t sourceOffset, uint64_t sourceSize, uint64_t alignment) noexcept;
        [[nodiscard]] bool copyIntoDestination(CommandBufferResource* commandBuffer, std::span<StagingTextureIndex const> stagingIndices, std::span<TextureUploadRegion const> regions, TextureResource* destination) noexcept;
        void freeBuffers() noexcept;

    private:

        BufferHandle createStagingBuffer(uint64_t size) noexcept;

        RendererContext* m_pContext{ nullptr };
        BufferResource* m_pFixedBuffer{ nullptr };
        uint64_t m_fixedBufferSize{ 0ull };
        uint64_t m_fixedHead{ 0ull };

        std::vector<BufferHandle> m_overflowBuffers;
    };
}

#endif