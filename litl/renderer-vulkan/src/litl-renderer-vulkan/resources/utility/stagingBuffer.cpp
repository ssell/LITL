#include "litl-core/assert.hpp"
#include "litl-renderer-vulkan/resources/utility/stagingBuffer.hpp"
#include "litl-renderer-vulkan/rendererContext.hpp"

namespace litl::vulkan
{
    StagingBuffer::StagingBuffer() 
        : m_pContext(nullptr), m_pFixedBuffer(nullptr), m_fixedHead(0u)
    {
        m_overflowBuffers.reserve(32ull);
    }

    StagingBuffer::~StagingBuffer()
    {

    }

    void StagingBuffer::build(RendererContext& context) noexcept
    {
        LITL_FATAL_ASSERT_MSG((context.config.stagingBufferFixedSize > 0u), "Renderer staging buffer fixed buffer size set to 0.");
        m_fixedBufferSize = context.config.stagingBufferFixedSize;
        m_pContext = &context;
        m_pFixedBuffer = m_pContext->resources.getBuffer(createStagingBuffer(m_fixedBufferSize));
        LITL_FATAL_ASSERT_MSG((m_pFixedBuffer != nullptr), "Failed to create fixed staging buffer for StagingBuffer");
    }

    std::optional<StagingBufferIndex> StagingBuffer::copyIntoStaging(std::span<std::byte const> source, uint64_t sourceOffset) noexcept
    {
        // 1. Allocate staging buffer

        BufferResource* targetBuffer = m_pFixedBuffer;

        StagingBufferIndex stagingIndex{
            .bufferOffset = m_fixedHead,
            .bufferSize = static_cast<uint64_t>(source.size()),
            .bufferIndex = StagingBufferIndex::FixedStagingBufferIndex
        };

        if ((m_fixedHead + stagingIndex.bufferSize) >= m_fixedBufferSize)
        {
            // No room in the fixed buffer for the source data. Allocate a temporary staging buffer to overflow into.
            BufferHandle tempStagingBufferHandle = createStagingBuffer(stagingIndex.bufferSize);
            targetBuffer = m_pContext->resources.getBuffer(tempStagingBufferHandle);

            LITL_ASSERT_MSG((targetBuffer != nullptr), "Failed to allocate temporary staging buffer for StagingBuffer", std::nullopt);

            stagingIndex.bufferIndex = static_cast<uint32_t>(m_overflowBuffers.size());
            stagingIndex.bufferOffset = 0ull;
            m_overflowBuffers.push_back(tempStagingBufferHandle);
        }
        else
        {
            // Room in the fixed buffer for the allocation. Increment the fixed head index.
            m_fixedHead += stagingIndex.bufferSize;
        }
        
        // 2. Copy into staging buffer

        const auto result = vmaCopyMemoryToAllocation(
            m_pContext->device.vmaAllocator,
            source.data() + sourceOffset,
            targetBuffer->allocation,
            static_cast<VkDeviceSize>(stagingIndex.bufferOffset),
            static_cast<VkDeviceSize>(source.size()));

        LITL_ASSERT_MSG((result == VK_SUCCESS), "Failed to copy source memory into staging buffer", std::nullopt);

        return stagingIndex;
    }

    bool StagingBuffer::copyIntoDestination(CommandBufferResource* commandBuffer, StagingBufferIndex stagingIndex, BufferResource* destination, uint64_t destOffset) noexcept
    {
        LITL_ASSERT_MSG((commandBuffer != nullptr), "Invalid command buffer provided to StagingBuffer::copyIntoDestination", false);

        VkBufferCopy bufferCopy{
            .srcOffset = static_cast<VkDeviceSize>(stagingIndex.bufferOffset),
            .dstOffset = static_cast<VkDeviceSize>(destOffset),
            .size = static_cast<VkDeviceSize>(stagingIndex.bufferSize)
        };

        BufferResource* sourceBuffer = m_pFixedBuffer;

        if (stagingIndex.bufferIndex != StagingBufferIndex::FixedStagingBufferIndex)
        {
            // Source data lies in an overflow buffer.
            LITL_ASSERT_MSG(stagingIndex.bufferIndex < static_cast<uint32_t>(m_overflowBuffers.size()), "Invalid overflow buffer index for StagingBuffer", false);
            BufferHandle sourceBufferHandle = m_overflowBuffers[stagingIndex.bufferIndex];
            sourceBuffer = m_pContext->resources.getBuffer(sourceBufferHandle);
            LITL_ASSERT_MSG((sourceBuffer != nullptr), "Invalid overflow buffer retrieved for StagingBuffer", false);
        }

        vkCmdCopyBuffer(commandBuffer->vkCommandBuffer, sourceBuffer->vkBuffer, destination->vkBuffer, 1, &bufferCopy);
        sourceBuffer->accumulatedDstStageMask |= deriveDstStageFromBufferType(destination->descriptor.type);                // record the accumulation on the staging buffer to be flushed at end of buffer upload scope (controlled by user)

        return true;
    }

    void StagingBuffer::flushBuffers(CommandBufferResource* commandBuffer) noexcept
    {
        LITL_ASSERT_MSG((commandBuffer != nullptr), "Invalid command buffer provided to StagingBuffer::flushBuffers", );

        flushBuffer(commandBuffer, m_pFixedBuffer);

        for (auto& overflowHandle : m_overflowBuffers)
        {
            flushBuffer(commandBuffer, m_pContext->resources.getBuffer(overflowHandle));
        }
    }

    void StagingBuffer::flushBuffer(CommandBufferResource* commandBuffer, BufferResource* buffer) noexcept
    {
        if ((buffer == nullptr) || (buffer->accumulatedDstStageMask == VK_PIPELINE_STAGE_2_NONE))
        {
            return;
        }

        const VkMemoryBarrier memoryBarrier{
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = static_cast<VkAccessFlags>(deriveDstAccessFromBufferType(buffer->descriptor.type))
        };


        vkCmdPipelineBarrier(
            commandBuffer->vkCommandBuffer,     // command buffer
            VK_PIPELINE_STAGE_TRANSFER_BIT,     // source stage mask
            buffer->accumulatedDstStageMask,    // dest stage mask
            0,                                  // dependency flags
            1,                                  // memory barrier count
            &memoryBarrier,                     // memory barriers
            0,                                  // buffer memory barrier count
            nullptr,                            // buffer memory barriers
            0,                                  // image memory barrier count
            nullptr);                           // image memory barriers
        
        buffer->accumulatedDstStageMask = VK_PIPELINE_STAGE_2_NONE;
    }

    void StagingBuffer::freeBuffers() noexcept
    {
        m_fixedHead = 0u;

        for (auto& bufferHandle : m_overflowBuffers)
        {
            m_pContext->resources.destroyBuffer(bufferHandle);
        }
    }

    BufferHandle StagingBuffer::createStagingBuffer(uint64_t size) noexcept
    {
        BufferDescriptor descriptor {
            .type = BufferTypeFlagBits::TransferSource,
            .memoryUsage = BufferMemoryUsage::Staging,
            .bytes = size
        };

        return m_pContext->resources.createBuffer(descriptor);
    }
}