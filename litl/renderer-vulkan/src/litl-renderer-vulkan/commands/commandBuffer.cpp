#include <algorithm>
#include <vector>

#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/commands/commandBuffer.hpp"

namespace LITL::Vulkan::Renderer
{
    struct CommandBufferHandle
    {
        VkDevice vkDevice;
        VkCommandPool vkCommandPool;
        uint32_t currFrame;
        std::vector<VkCommandBuffer> vkCommandBuffers;
    };

    std::unique_ptr<LITL::Renderer::CommandBuffer> createCommandBuffer(VkDevice vkDevice, VkCommandPool vkCommandPool, uint32_t framesInFlight)
    {
        auto* handle = new CommandBufferHandle{ 
            vkDevice, 
            vkCommandPool, 
            0,
            std::vector<VkCommandBuffer>{ framesInFlight } 
        };

        std::fill(handle->vkCommandBuffers.begin(), handle->vkCommandBuffers.end(), VK_NULL_HANDLE);

        return std::make_unique<LITL::Renderer::CommandBuffer>(
            &VulkanCommandBufferOperations,
            LITL_PACK_HANDLE(LITL::Renderer::CommandBufferHandle, handle)
        );
    }

    VkCommandBuffer getCurrentCommandBuffer(CommandBufferHandle const* handle)
    {
        return handle->vkCommandBuffers[handle->currFrame % handle->vkCommandBuffers.size()];
    }

    bool build(LITL::Renderer::CommandBufferHandle const& litlHandle)
    {
        auto* handle = LITL_UNPACK_HANDLE(CommandBufferHandle, litlHandle);

        if (handle->vkCommandBuffers[0] != VK_NULL_HANDLE)
        {
            logWarning("Requesting to build Vulkan Command Buffer that is already built.");
            return false;
        }

        const auto info = VkCommandBufferAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = handle->vkCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,               // Can be submitted to a queue for execution, but cannot be called from other command buffers.
            .commandBufferCount = static_cast<uint32_t>(handle->vkCommandBuffers.size())
        };

        VkResult result = vkAllocateCommandBuffers(handle->vkDevice, &info, handle->vkCommandBuffers.data());

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Command Buffer(s) with result ", result);
            return false;
        }

        return true;
    }

    void destroy(LITL::Renderer::CommandBufferHandle const& litlHandle)
    {
        auto* handle = LITL_UNPACK_HANDLE(CommandBufferHandle, litlHandle);

        if (handle->vkCommandBuffers[0] != VK_NULL_HANDLE)
        {
            vkFreeCommandBuffers(handle->vkDevice, handle->vkCommandPool, handle->vkCommandBuffers.size(), handle->vkCommandBuffers.data());
            std::fill(handle->vkCommandBuffers.begin(), handle->vkCommandBuffers.end(), VK_NULL_HANDLE);
        }
    }

    bool begin(LITL::Renderer::CommandBufferHandle const& litlHandle, uint32_t frame)
    {
        auto* handle = LITL_UNPACK_HANDLE(CommandBufferHandle, litlHandle);

        handle->currFrame = frame;

        const auto info = VkCommandBufferBeginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
            // .flags = ...; // (Optional)
        };

        const auto result = vkBeginCommandBuffer(getCurrentCommandBuffer(handle), &info);

        if (result != VK_SUCCESS)
        {
            logError("Failed to begin recording Vulkan Command Buffer with result ", result);
            return false;
        }

        return true;
    }

    bool end(LITL::Renderer::CommandBufferHandle const& litlHandle)
    {
        auto* handle = LITL_UNPACK_HANDLE(CommandBufferHandle, litlHandle);

        const auto result = vkEndCommandBuffer(getCurrentCommandBuffer(handle));

        if (result != VK_SUCCESS)
        {
            logError("Failedto end recording Vulkan Command Buffer with result ", result);
        }

        return true;
    }

    VkCommandBuffer extractCurrentVkCommandBuffer(LITL::Renderer::CommandBuffer const* commandBuffer)
    {
        auto* handle = LITL_UNPACK_HANDLE_PTR(CommandBufferHandle, commandBuffer->getHandle());
        return getCurrentCommandBuffer(handle);
    }
}