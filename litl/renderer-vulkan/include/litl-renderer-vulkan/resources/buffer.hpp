#ifndef LITL_RENDERER_VULKAN_BUFFER_H__
#define LITL_RENDERER_VULKAN_BUFFER_H__

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/buffer.hpp"

namespace litl::vulkan
{
    struct BufferMemoryMap
    {
        /// <summary>
        /// 
        /// </summary>
        void* persistent = nullptr;

        /// <summary>
        /// 
        /// </summary>
        void* temporary = nullptr;

        /// <summary>
        /// If the buffer is made with type ShaderDeviceAddress this is the buffer device address.
        /// </summary>
        VkDeviceAddress bdaAddress = 0;
    };

    struct BufferResource
    {
        /// <summary>
        /// The Vulkan buffer handle.
        /// </summary>
        VkBuffer vkBuffer = VK_NULL_HANDLE;

        /// <summary>
        /// The VMA allocated buffer memory.
        /// </summary>
        VmaAllocation allocation = VK_NULL_HANDLE;

        /// <summary>
        /// Optional pointer to persistent mapped memory, offset, etc.
        /// </summary>
        VmaAllocationInfo allocationInfo{};

        /// <summary>
        /// ...
        /// </summary>
        BufferMemoryMap memoryMap{};

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

        /// <summary>
        /// The descriptor that created the buffer.
        /// </summary>
        BufferDescriptor descriptor{};
    };
}

#endif