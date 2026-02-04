#include <algorithm>

#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/commands/commandBuffer.hpp"

namespace LITL::Vulkan::Renderer
{
    CommandBuffer::CommandBuffer(VkDevice vkDevice, VkCommandPool vkCommandPool, uint32_t framesInFlight)
        : m_vkDevice(vkDevice), m_vkCommandPool(vkCommandPool), m_vkCommandBuffers(std::vector<VkCommandBuffer>{ framesInFlight })
    {
        std::fill(m_vkCommandBuffers.begin(), m_vkCommandBuffers.end(), VK_NULL_HANDLE);
    }

    CommandBuffer::~CommandBuffer()
    {
        if (m_vkCommandBuffers[0] != VK_NULL_HANDLE)
        {
            vkFreeCommandBuffers(m_vkDevice, m_vkCommandPool, m_vkCommandBuffers.size(), m_vkCommandBuffers.data());
        }
    }

    bool CommandBuffer::build()
    {
        if (m_vkCommandBuffers[0] != VK_NULL_HANDLE)
        {
            logWarning("Requesting to build Vulkan Command Buffer that is already built.");
            return false;
        }

        const auto info = VkCommandBufferAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_vkCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,               // Can be submitted to a queue for execution, but cannot be called from other command buffers.
            .commandBufferCount = static_cast<uint32_t>(m_vkCommandBuffers.size())
        };

        VkResult result = vkAllocateCommandBuffers(m_vkDevice, &info, m_vkCommandBuffers.data());

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Command Buffer(s) with result ", result);
            return false;
        }

        return true;
    }

    bool CommandBuffer::begin(uint32_t frame)
    {
        const auto info = VkCommandBufferBeginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
            // .flags = ...; // (Optional)
        };

        VkResult result = vkBeginCommandBuffer(getCurrentCommandBuffer(frame), &info);

        if (result != VK_SUCCESS)
        {
            // todo log out error
            return false;
        }

        return true;
    }

    bool CommandBuffer::end(uint32_t frame)
    {
        VkResult result = vkEndCommandBuffer(getCurrentCommandBuffer(frame));

        if (result != VK_SUCCESS)
        {
            // todo log out error
            return false;
        }

        return true;
    }

    VkCommandBuffer CommandBuffer::getCurrentCommandBuffer(uint32_t frame) const noexcept
    {
        return m_vkCommandBuffers[frame % m_vkCommandBuffers.size()];
    }
}