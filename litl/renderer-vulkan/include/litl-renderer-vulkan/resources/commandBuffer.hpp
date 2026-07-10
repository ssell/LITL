#ifndef LITL_RENDERER_VULKAN_COMMAND_BUFFER_H__
#define LITL_RENDERER_VULKAN_COMMAND_BUFFER_H__

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/commandBuffer.hpp"
#include "litl-renderer/resources/graphicsPipeline.hpp"
#include "litl-renderer/resources/computePipeline.hpp"
#include "litl-renderer-vulkan/resources/utility/descriptorSetChangeTracker.hpp"

namespace litl::vulkan
{
    struct CommandBufferResource
    {
        /// <summary>
        /// The Vulkan command buffer.
        /// </summary>
        VkCommandBuffer vkCommandBuffer = VK_NULL_HANDLE;

        /// <summary>
        /// The command pool that created the buffer.
        /// </summary>
        VkCommandPool vkCommandPool = VK_NULL_HANDLE;

        /// <summary>
        /// Is this a short-lived buffer?
        /// </summary>
        bool isTransient = false;

        /// <summary>
        /// The currently bound graphics pipeline (if any).
        /// </summary>
        GraphicsPipelineHandle boundGraphicsPipeline{};

        /// <summary>
        /// The currently bound compute pipeline (if any).
        /// </summary>
        ComputePipelineHandle boundComputePipeline{};

        /// <summary>
        /// Accumulated descriptor set changes.
        /// </summary>
        DescriptorSetChangeTracker descriptorSetChanges;
    };
}

#endif