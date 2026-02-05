#ifndef LITL_VULKAN_RENDERER_COMMAND_BUFFER_H__
#define LITL_VULKAN_RENDERER_COMMAND_BUFFER_H__

#include <vulkan/vulkan.h>
#include "litl-renderer/commands/commandBuffer.hpp"

namespace LITL::Vulkan::Renderer
{
    std::unique_ptr<LITL::Renderer::CommandBuffer> createCommandBuffer(VkDevice vkDevice, VkCommandPool vkCommandPool, uint32_t framesInFlight);

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