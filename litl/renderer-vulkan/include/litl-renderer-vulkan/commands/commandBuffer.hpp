#ifndef LITL_VULKAN_RENDERER_COMMAND_BUFFER_H__
#define LITL_VULKAN_RENDERER_COMMAND_BUFFER_H__

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

#include "litl-renderer/commands/commandBuffer.hpp"

namespace LITL::Vulkan::Renderer
{
    struct CommandBufferHandle
    {
        VkDevice vkDevice;
        VkCommandPool vkCommandPool;
        uint32_t currFrame;
        std::vector<VkCommandBuffer> vkCommandBuffers;
    };

    /// <summary>
    /// Instantiates a new CommandBuffer and returns the raw owning pointer to it.
    /// </summary>
    /// <param name="vkDevice"></param>
    /// <param name="vkCommandPool"></param>
    /// <param name="currFrame"></param>
    /// <param name="framesInFlight"></param>
    /// <returns></returns>
    LITL::Renderer::CommandBuffer* createCommandBuffer(VkDevice vkDevice, VkCommandPool vkCommandPool, uint32_t currFrame, uint32_t framesInFlight);

    VkCommandBuffer extractCurrentVkCommandBuffer(LITL::Renderer::CommandBuffer const* commandBuffer);

    bool build(LITL::Renderer::CommandBufferHandle const& litlCmdBufferHandle);
    void destroy(LITL::Renderer::CommandBufferHandle const& litlCmdBufferHandle);
    bool begin(LITL::Renderer::CommandBufferHandle const& litlCmdBufferHandle, uint32_t frame);
    bool end(LITL::Renderer::CommandBufferHandle const& litlCmdBufferHandle);

    void cmdBeginRenderPass(LITL::Renderer::CommandBufferHandle const& litlCmdBufferHandle, LITL::Renderer::RendererHandle const* pLitlRendererHandle, uint32_t swapChainIndex);
    void cmdEndRenderPass(LITL::Renderer::CommandBufferHandle const& litlCmdBufferHandle);
    void cmdBindGraphicsPipeline(LITL::Renderer::CommandBufferHandle const& litlCbHandle, LITL::Renderer::GraphicsPipelineHandle const& litlGraphicsHandle);

    const LITL::Renderer::CommandBufferOperations VulkanCommandBufferOperations = {
        &build,
        &destroy,
        &begin,
        &end
    };
}

#endif