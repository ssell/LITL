#include "litl-core/assert.hpp"
#include "litl-renderer-vulkan/resources/utility/stagingTexture.hpp"
#include "litl-renderer-vulkan/rendererContext.hpp"

namespace litl::vulkan
{
    StagingTexture::StagingTexture()
        : m_pContext(nullptr)
    {
        m_overflowBuffers.reserve(32ull);
    }

    StagingTexture::~StagingTexture()
    {

    }

    void StagingTexture::build(RendererContext& context) noexcept
    {
        LITL_FATAL_ASSERT_MSG((context.config.stagingTextureFixedSize > 0u), "Renderer staging texture fixed buffer size set to 0.");
        m_fixedBufferSize = context.config.stagingBufferFixedSize;
        m_pContext = &context;
        m_pFixedBuffer = m_pContext->resources.getBuffer(createStagingBuffer(m_fixedBufferSize));
        LITL_FATAL_ASSERT_MSG((m_pFixedBuffer != nullptr), "Failed to create fixed staging buffer for StagingTexture");
    }

    std::optional<StagingTextureIndex> StagingTexture::copyIntoStaging(std::span<std::byte const> source, uint64_t sourceOffset) noexcept
    {
        // 1. Allocate staging texture

        BufferResource* targetBuffer = m_pFixedBuffer;

        StagingTextureIndex stagingIndex{
            .bufferOffset = m_fixedHead,
            .bufferSize = static_cast<uint64_t>(source.size()),
            .bufferIndex = StagingTextureIndex::FixedStagingTextureIndex
        };

        if ((m_fixedHead + stagingIndex.bufferSize) >= m_fixedBufferSize)
        {
            // No room in the fixed buffer for the source data. Allocate a temporary staging buffer to overflow into.
            BufferHandle tempStagingBufferHandle = createStagingBuffer(stagingIndex.bufferSize);
            targetBuffer = m_pContext->resources.getBuffer(tempStagingBufferHandle);

            LITL_ASSERT_MSG((targetBuffer != nullptr), "Failed to allocate temporary staging buffer for StagingTexture", std::nullopt);

            stagingIndex.bufferIndex = static_cast<uint32_t>(m_overflowBuffers.size());
            stagingIndex.bufferOffset = 0ull;
            m_overflowBuffers.push_back(tempStagingBufferHandle);
        }
        else
        {
            // Room in the fixed buffer for the allocation. Increment the fixed head index.
            m_fixedHead += stagingIndex.bufferSize;
        }

        // 2. Copy into staging texture

        const auto result = vmaCopyMemoryToAllocation(
            m_pContext->device.vmaAllocator,
            source.data() + sourceOffset,
            targetBuffer->allocation,
            static_cast<VkDeviceSize>(stagingIndex.bufferOffset),
            static_cast<VkDeviceSize>(source.size()));

        LITL_ASSERT_MSG((result == VK_SUCCESS), "Failed to copy source memory into staging buffer", std::nullopt);

        return stagingIndex;
    }

    bool StagingTexture::copyIntoDestination(CommandBufferResource* commandBuffer, StagingTextureIndex stagingIndex, TextureResource* destination) noexcept
    {
        LITL_ASSERT_MSG((commandBuffer != nullptr), "Invalid command buffer provided to StagingTexture::copyIntoDestination", false);

        BufferResource* sourceBuffer = m_pFixedBuffer;

        if (stagingIndex.bufferIndex != StagingTextureIndex::FixedStagingTextureIndex)
        {
            // Source data lies in an oveflow buffer.
            LITL_ASSERT_MSG(stagingIndex.bufferIndex < static_cast<uint32_t>(m_overflowBuffers.size()), "Invalid overflow buffer index for StagingTexture", false);
            BufferHandle sourceBufferHandle = m_overflowBuffers[stagingIndex.bufferIndex];
            sourceBuffer = m_pContext->resources.getBuffer(sourceBufferHandle);
            LITL_ASSERT_MSG((sourceBuffer != nullptr), "Invalid overflow buffer retrieved for StagingTexture", false);
        }

        VkBufferImageCopy2 copyRegion{
            .sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = VkImageSubresourceLayers {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,        // todo update to support depth, etc.
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 0
            },
            .imageExtent = VkExtent3D {
                .width = destination->descriptor.width,
                .height = destination->descriptor.height,
                .depth = destination->descriptor.depth
            }
        };

        VkCopyBufferToImageInfo2 copyInfo{
            .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
            .srcBuffer = nullptr,
            .dstImage = destination->vkImage,
            .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .regionCount = 1,
            .pRegions = &copyRegion
        };

        vkCmdCopyBufferToImage2(commandBuffer->vkCommandBuffer, &copyInfo);
        sourceBuffer->accumulatedDstStageMask |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;     // todo, prob wont work for compute (do compute shaders even use textures?)

        return true;
    }

    void StagingTexture::flushBuffers(CommandBufferResource* commandBuffer) noexcept
    {
        LITL_ASSERT_MSG((commandBuffer != nullptr), "Invalid command buffer provided to StagingTexture::flushTextures", );

        flushBuffer(commandBuffer, m_pFixedBuffer);

        for (auto& overflowHandle : m_overflowBuffers)
        {
            flushBuffer(commandBuffer, m_pContext->resources.getBuffer(overflowHandle));
        }
    }

    void StagingTexture::flushBuffer(CommandBufferResource* commandBuffer, BufferResource* buffer) noexcept
    {
        if ((buffer == nullptr) || (buffer->accumulatedDstStageMask == VK_PIPELINE_STAGE_2_NONE))
        {
            return;
        }

        const VkImageMemoryBarrier2 imageBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
            .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .dstStageMask = buffer->accumulatedDstStageMask,
            .dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        const VkDependencyInfo dependencyInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &imageBarrier
        };

        vkCmdPipelineBarrier2(commandBuffer->vkCommandBuffer, &dependencyInfo);
    }

    void StagingTexture::freeBuffers() noexcept
    {
        m_fixedHead = 0u;

        for (auto& bufferHandle : m_overflowBuffers)
        {
            m_pContext->resources.destroyBuffer(bufferHandle);
        }
    }

    BufferHandle StagingTexture::createStagingBuffer(uint64_t size) noexcept
    {
        BufferDescriptor descriptor{
            .type = BufferTypeFlagBits::TransferSource,
            .memoryUsage = BufferMemoryUsage::Staging,
            .bytes = size
        };

        return m_pContext->resources.createBuffer(descriptor);
    }
}