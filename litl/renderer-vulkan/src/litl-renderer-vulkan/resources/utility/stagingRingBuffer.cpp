#include "litl-core/assert.hpp"
#include "litl-renderer-vulkan/resources/utility/stagingRingBuffer.hpp"
#include "litl-renderer-vulkan/rendererContext.hpp"

namespace litl::vulkan
{
    StagingRingBuffer::StagingRingBuffer() 
        : m_pContext(nullptr), m_pFixedBuffer(nullptr), m_fixedHead(0ull)
    {
        m_overflowBuffers.resize(32ull);
    }

    StagingRingBuffer::~StagingRingBuffer()
    {

    }

    void StagingRingBuffer::build(RendererContext& context) noexcept
    {
        m_pContext = &context;
        m_pFixedBuffer = m_pContext->resources.getBuffer(createStagingBuffer(FixedBufferSize));
        LITL_FATAL_ASSERT_MSG((m_pFixedBuffer != nullptr), "Failed to create fixed staging buffer for StagingRingBuffer");
    }

    StagingRingBufferIndex StagingRingBuffer::copyIntoStaging(std::span<std::byte const> source, uint64_t sourceOffset) noexcept
    {
        BufferResource* targetBuffer = m_pFixedBuffer;
        VkDeviceSize targetOffset = static_cast<VkDeviceSize>(m_fixedHead);

        StagingRingBufferIndex stagingIndex{
            .bufferIndex = StagingRingBufferIndex::FixedRingBufferIndex,
            .bufferOffset = m_fixedHead,
            .bufferSize = static_cast<uint64_t>(source.size())
        };

        if ((m_fixedHead + stagingIndex.bufferSize) >= FixedBufferSize)
        {
            // No room in the fixed buffer for the source data. Allocate a temporary staging buffer to overflow into.
            BufferHandle tempStagingBufferHandle = createStagingBuffer(stagingIndex.bufferSize);
            targetBuffer = m_pContext->resources.getBuffer(tempStagingBufferHandle);

            LITL_ASSERT_MSG((targetBuffer != nullptr), "Failed to allocate temporary staging buffer for StagingRingBuffer", {});

            stagingIndex.bufferIndex = static_cast<uint32_t>(m_overflowBuffers.size());
            stagingIndex.bufferOffset = 0ull;
            m_overflowBuffers.push_back(tempStagingBufferHandle);
        }
        else
        {
            // Room in the fixed buffer for the allocation. Increment the fixed head index.
            m_fixedHead += stagingIndex.bufferSize;
        }

        vmaCopyMemoryToAllocation(
            m_pContext->device.vmaAllocator,
            source.data() + sourceOffset,
            targetBuffer->allocation,
            static_cast<VkDeviceSize>(stagingIndex.bufferOffset),
            static_cast<VkDeviceSize>(source.size()));

        return stagingIndex;
    }

    void StagingRingBuffer::copyIntoDestination(CommandBufferResource* commandBuffer, StagingRingBufferIndex stagingIndex, BufferResource* destination, uint64_t destOffset) noexcept
    {
        LITL_ASSERT_MSG(commandBuffer != nullptr, "Invalid command buffer provided to StagingRingBuffer::copyIntoDestination", );

        VkBufferCopy bufferCopy{
            .srcOffset = static_cast<VkDeviceSize>(stagingIndex.bufferOffset),
            .dstOffset = static_cast<VkDeviceSize>(destOffset),
            .size = static_cast<VkDeviceSize>(stagingIndex.bufferSize)
        };

        VkBuffer sourceVkBuffer = m_pFixedBuffer->vkBuffer;

        if (stagingIndex.bufferIndex != StagingRingBufferIndex::FixedRingBufferIndex)
        {
            // Source data lies in an overflow buffer.
            LITL_ASSERT_MSG(stagingIndex.bufferIndex < static_cast<uint32_t>(m_overflowBuffers.size()), "Invalid overflow buffer index for StagingRingBuffer", );
            BufferHandle sourceBufferHandle = m_overflowBuffers[stagingIndex.bufferIndex];
            BufferResource* sourceBuffer = m_pContext->resources.getBuffer(sourceBufferHandle);
            LITL_ASSERT_MSG((sourceBuffer != nullptr), "Invalid overflow buffer retrieved for StagingRingBuffer", );

            sourceVkBuffer = sourceBuffer->vkBuffer;
        }

        vkCmdCopyBuffer(commandBuffer->vkCommandBuffer, sourceVkBuffer, destination->vkBuffer, 1, &bufferCopy);
        destination->accumulatedDstStageMask |= deriveDstStageFromBufferType(destination->descriptor.type);
    }

    void StagingRingBuffer::flushBuffers(CommandBufferResource* commandBuffer)
    {
        LITL_ASSERT_MSG(commandBuffer != nullptr, "Invalid command buffer provided to StagingRingBuffer::flushBuffers", );

        flushBuffer(commandBuffer, m_pFixedBuffer);

        for (auto& overflowHandle : m_overflowBuffers)
        {
            flushBuffer(commandBuffer, m_pContext->resources.getBuffer(overflowHandle));
        }
    }

    void StagingRingBuffer::flushBuffer(CommandBufferResource* commandBuffer, BufferResource* buffer) noexcept
    {
        if ((buffer == nullptr) || (buffer->accumulatedDstStageMask == VK_PIPELINE_STAGE_2_NONE))
        {
            return;
        }

        const VkBufferMemoryBarrier memoryBarrier{
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = static_cast<VkPipelineStageFlags>(deriveDstStageFromBufferType(buffer->descriptor.type)),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = buffer->vkBuffer,
            .offset = 0,
            .size = buffer->allocationInfo.size
        };


        vkCmdPipelineBarrier(
            commandBuffer->vkCommandBuffer,     // command buffer
            VK_PIPELINE_STAGE_TRANSFER_BIT,     // source stage mask
            buffer->accumulatedDstStageMask,    // dest stage mask
            0,                                  // dependency flags
            0,                                  // memory barrier count
            nullptr,                            // memory barriers
            1,                                  // buffer memory barrier count
            &memoryBarrier,                     // buffer memory barriers
            0,                                  // image memory barrier count
            nullptr);                           // image memory barriers
        

        buffer->accumulatedDstStageMask = VK_PIPELINE_STAGE_2_NONE;
    }

    void StagingRingBuffer::freeBuffers() noexcept
    {
        m_fixedHead = 0ull;

        for (auto& bufferHandle : m_overflowBuffers)
        {
            m_pContext->resources.destroyBuffer(bufferHandle);
        }
    }

    BufferHandle StagingRingBuffer::createStagingBuffer(uint64_t size) noexcept
    {
        BufferDescriptor descriptor {
            .type = BufferTypeFlagBits::TransferSource,
            .memoryUsage = BufferMemoryUsage::Staging,
            .bytes = size
        };

        return m_pContext->resources.createBuffer(descriptor);
    }
}