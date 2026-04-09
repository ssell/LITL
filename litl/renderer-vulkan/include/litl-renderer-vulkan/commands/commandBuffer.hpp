#ifndef LITL_VULKAN_RENDERER_COMMAND_BUFFER_H__
#define LITL_VULKAN_RENDERER_COMMAND_BUFFER_H__

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

#include "litl-renderer/commands/commandBuffer.hpp"

namespace litl::vulkan
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
    litl::CommandBuffer* createCommandBuffer(VkDevice vkDevice, VkCommandPool vkCommandPool, uint32_t currFrame, uint32_t framesInFlight);

    VkCommandBuffer extractCurrentVkCommandBuffer(litl::CommandBuffer const* commandBuffer);

    bool build(litl::CommandBufferHandle const& litlCmdBufferHandle);
    void destroy(litl::CommandBufferHandle const& litlCmdBufferHandle);
    bool begin(litl::CommandBufferHandle const& litlCmdBufferHandle, uint32_t frame);
    bool end(litl::CommandBufferHandle const& litlCmdBufferHandle);

    void cmdBeginRenderPass(litl::CommandBufferHandle const& litlCmdBufferHandle, litl::RendererHandle const* pLitlRendererHandle, uint32_t swapChainIndex);
    void cmdEndRenderPass(litl::CommandBufferHandle const& litlCmdBufferHandle);
    void cmdBindGraphicsPipeline(litl::CommandBufferHandle const& litlCbHandle, litl::GraphicsPipelineHandle const& litlGraphicsHandle);

    const litl::CommandBufferOperations VulkanCommandBufferOperations = {
        &build,
        &destroy,
        &begin,
        &end
    };
}

#endif