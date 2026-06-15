#ifndef LITL_RENDERER_VULKAN_COMMAND_BUFFER_H__
#define LITL_RENDERER_VULKAN_COMMAND_BUFFER_H__

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/commandBuffer.hpp"
#include "litl-renderer/resources/graphicsPipeline.hpp"
#include "litl-renderer/resources/computePipeline.hpp"

namespace litl::vulkan
{
    struct CommandBufferResource
    {
        /// <summary>
        /// The Vulkan command buffer.
        /// </summary>
        VkCommandBuffer vkCommandBuffer = VK_NULL_HANDLE;

        /// <summary>
        /// The currently bound graphics pipeline (if any).
        /// </summary>
        GraphicsPipelineHandle boundGraphicsPipeline{};

        /// <summary>
        /// The currently bound compute pipeline (if any).
        /// </summary>
        ComputePipelineHandle boundComputePipeline{};
    };
}

#endif