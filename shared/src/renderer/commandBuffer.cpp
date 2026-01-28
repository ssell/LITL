#include "renderer/common.inl"
#include "renderer/commandBuffer.hpp"

namespace LITL
{
    // https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html

    CommandBuffer::CommandBuffer(RenderContext const* const pRenderContext)
        : m_pContext(new CommandBufferContext{})
    {
        m_pContext->device = pRenderContext->device;
        m_pContext->commandPool = pRenderContext->commandPool;
    }

    CommandBuffer::~CommandBuffer()
    {
        for (size_t i = 0; i < m_pContext->commandBuffers.size(); ++i)
        {
            if (m_pContext->commandBuffers[i] != VK_NULL_HANDLE)
            {
                vkFreeCommandBuffers(m_pContext->device, m_pContext->commandPool, 1, &m_pContext->commandBuffers[i]);
            }

            m_pContext->commandBuffers.clear();
        }

        delete m_pContext;
    }

    bool CommandBuffer::build()
    {
        if (m_pContext->commandBuffers.size() > 0)
        {
            return false;
        }

        const auto info = VkCommandBufferAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_pContext->commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,               // Can be submitted to a queue for execution, but cannot be called from other command buffers.
            .commandBufferCount = FRAMES_IN_FLIGHT
        };

        m_pContext->commandBuffers.resize(FRAMES_IN_FLIGHT, VK_NULL_HANDLE);

        VkResult result = vkAllocateCommandBuffers(m_pContext->device, &info, &m_pContext->commandBuffers[0]);

        if (result != VK_SUCCESS)
        {
            // todo log out error
            return false;
        }

        return true;
    }

    CommandBufferContext const* CommandBuffer::getCommandBufferContext() const
    {
        return m_pContext;
    }

    bool CommandBuffer::begin(RenderContext const* const pRenderContext) const
    {
        const auto info = VkCommandBufferBeginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
            // .flags = ...; // (Optional)
        };

        VkResult result = vkBeginCommandBuffer(m_pContext->getCurrentCommandBuffer(pRenderContext), &info);
        
        if (result != VK_SUCCESS)
        {
            // todo log out error
            return false;
        }

        return true;
    }

    bool CommandBuffer::end(RenderContext const* const pRenderContext) const
    {
        VkResult result = vkEndCommandBuffer(m_pContext->getCurrentCommandBuffer(pRenderContext));

        if (result != VK_SUCCESS)
        {
            // todo log out error
            return false;
        }

        return true;
    }

    void CommandBuffer::commandImageLayoutTransition(
        RenderContext const* const pRenderContext,
        uint32_t const imageIndex, 
        uint32_t const oldLayout, 
        uint32_t const newLayout, 
        uint32_t const srcAccessMask, 
        uint32_t const dstAccessMask, 
        uint32_t const srcStageMask, 
        uint32_t const dstStageMask) const
    {
        const auto barrier = VkImageMemoryBarrier2 {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = srcStageMask,
            .srcAccessMask = srcAccessMask,
            .dstStageMask = dstStageMask,
            .dstAccessMask = dstAccessMask,
            .oldLayout = static_cast<VkImageLayout>(oldLayout),
            .newLayout = static_cast<VkImageLayout>(newLayout),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = pRenderContext->swapChainImages[imageIndex],
            .subresourceRange = VkImageSubresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        const auto info = VkDependencyInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags = {},
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier
        };
        
        vkCmdPipelineBarrier2(m_pContext->getCurrentCommandBuffer(pRenderContext), &info);
    }
}