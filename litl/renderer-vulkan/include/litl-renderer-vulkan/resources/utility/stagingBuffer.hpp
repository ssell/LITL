#ifndef LITL_RENDERER_VULKAN_STAGING_RING_BUFFER_H__
#define LITL_RENDERER_VULKAN_STAGING_RING_BUFFER_H__

#include <optional>
#include <span>
#include <vector>

#include "litl-core/constants.hpp"
#include "litl-renderer-vulkan/resources/buffer.hpp"
#include "litl-renderer-vulkan/resources/commandBuffer.hpp"

namespace litl::vulkan
{
    struct RendererContext;

    struct StagingBufferIndex
    {
        static const uint32_t FixedStagingBufferIndex = litl::Constants::uint32_null_index;

        uint64_t bufferOffset = 0ull;
        uint64_t bufferSize = 0ull;
        uint32_t bufferIndex = FixedStagingBufferIndex;
    };

    /// <summary>
    /// A memory arena for staging/transit buffers.
    /// Composed of a fixed size persistent buffer and a list of temporary overflow buffers.
    /// </summary>
    class StagingBuffer final
    {
    public:

        StagingBuffer();

        StagingBuffer(StagingBuffer const&) = delete;
        StagingBuffer& operator=(StagingBuffer const&) = delete;

        void build(RendererContext& context) noexcept;
        void destroy() noexcept;

        [[nodiscard]] std::optional<StagingBufferIndex> copyIntoStaging(std::span<std::byte const> source, uint64_t sourceOffset) noexcept;
        [[nodiscard]] bool copyIntoDestination(CommandBufferResource* commandBuffer, StagingBufferIndex stagingIndex, BufferResource* destination, uint64_t destOffset) noexcept;
        void flushBuffers(CommandBufferResource* commandBuffer) noexcept;
        void freeBuffers() noexcept;

    private:

        BufferHandle createStagingBuffer(uint64_t size) noexcept;
        void flushBuffer(CommandBufferResource* commandBuffer, BufferResource* resource) noexcept;

        RendererContext* m_pContext;
        BufferResource* m_pFixedBuffer;
        uint32_t m_fixedBufferSize;
        uint32_t m_fixedHead;

        std::vector<BufferHandle> m_overflowBuffers;
    };
}

#endif