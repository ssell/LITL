#ifndef LITL_RENDERER_VULKAN_BUFFER_H__
#define LITL_RENDERER_VULKAN_BUFFER_H__

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/buffer.hpp"

namespace litl::vulkan
{
    struct BufferResource
    {
        /// <summary>
        /// The descriptor that created the buffer.
        /// </summary>
        BufferDescriptor descriptor{};

        /// <summary>
        /// The Vulkan buffer handle.
        /// </summary>
        VkBuffer vkBuffer = VK_NULL_HANDLE;

        /// <summary>
        /// The VMA allocated buffer memory.
        /// </summary>
        VmaAllocation allocation = VK_NULL_HANDLE;

        /// <summary>
        /// Optional pointer to mapped memory, offset, etc.
        /// </summary>
        VmaAllocationInfo allocationInfo{};

        /// <summary>
        /// For persistently mapped buffers, they may require a dedicated staging buffer depending on where they were able to be allocated.
        /// </summary>
        BufferHandle stagingBuffer{};

        /// <summary>
        /// When buffers are copied via vkCmdCopyBuffer a memory barrier is required to ensure that the
        /// data is ready before it is attempted to be used. As different buffers have different usage
        /// requirements, this mask represents an accumulation of pipeline usage so that an accurate
        /// memory barrier may be constructed.
        /// </summary>
        VkPipelineStageFlags2 accumulatedDstStageMask = VK_PIPELINE_STAGE_2_NONE;
    };
}

#endif