#ifndef LITL_VULKAN_RENDERER_COMMAND_BUFFER_H__
#define LITL_VULKAN_RENDERER_COMMAND_BUFFER_H__

#include <vulkan/vulkan.h>
#include "litl-renderer/commands/commandBuffer.hpp"

namespace LITL::Vulkan::Renderer
{
    /// <summary>
    /// Instantiates a new CommandBuffer and returns the raw owning pointer to it.
    /// </summary>
    /// <param name="vkDevice"></param>
    /// <param name="vkCommandPool"></param>
    /// <param name="currFrame"></param>
    /// <param name="framesInFlight"></param>
    /// <returns></returns>
    LITL::Renderer::CommandBuffer* createCommandBuffer(VkDevice vkDevice, VkCommandPool vkCommandPool, uint32_t currFrame, uint32_t framesInFlight);

    bool build(LITL::Renderer::CommandBufferHandle const& litlHandle);
    void destroy(LITL::Renderer::CommandBufferHandle const& litlHandle);
    bool begin(LITL::Renderer::CommandBufferHandle const& litlHandle, uint32_t frame);
    bool end(LITL::Renderer::CommandBufferHandle const& litlHandle);

    VkCommandBuffer extractCurrentVkCommandBuffer(LITL::Renderer::CommandBuffer const* commandBuffer);

    const LITL::Renderer::CommandBufferOperations VulkanCommandBufferOperations = {
        &build,
        &destroy,
        &begin,
        &end
    };
}

#endif