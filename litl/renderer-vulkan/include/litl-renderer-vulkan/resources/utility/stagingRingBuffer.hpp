#ifndef LITL_RENDERER_VULKAN_STAGING_RING_BUFFER_H__
#define LITL_RENDERER_VULKAN_STAGING_RING_BUFFER_H__

#include <span>

#include "litl-core/constants.hpp"
#include "litl-renderer-vulkan/resources/buffer.hpp"
#include "litl-renderer-vulkan/resources/commandBuffer.hpp"

namespace litl::vulkan
{
    struct RendererContext;

    struct StagingRingBufferIndex
    {
        static const uint32_t FixedRingBufferIndex = litl::Constants::uint32_null_index;

        uint64_t bufferOffset = 0ull;
        uint64_t bufferSize = 0ull;
        uint32_t bufferIndex = FixedRingBufferIndex;
    };

    /// <summary>
    /// 
    /// </summary>
    class StagingRingBuffer final
    {
    public:

        static const uint64_t FixedBufferSize = 32768;

        StagingRingBuffer();
        ~StagingRingBuffer();

        StagingRingBuffer(StagingRingBuffer const&) = delete;
        StagingRingBuffer& operator=(StagingRingBuffer const&) = delete;

        void build(RendererContext& context) noexcept;

        StagingRingBufferIndex copyIntoStaging(std::span<std::byte const> source, uint64_t sourceOffset) noexcept;
        void copyIntoDestination(CommandBufferResource* commandBuffer, StagingRingBufferIndex stagingIndex, BufferResource* destination, uint64_t destOffset) noexcept;
        void flushBuffers(CommandBufferResource* commandBuffer);
        void freeBuffers() noexcept;

    private:

        BufferHandle createStagingBuffer(uint64_t size) noexcept;
        void flushBuffer(CommandBufferResource* commandBuffer, BufferResource* resource) noexcept;

        RendererContext* m_pContext;
        BufferResource* m_pFixedBuffer;
        uint64_t m_fixedHead;

        std::vector<BufferHandle> m_overflowBuffers;
    };
}

#endif