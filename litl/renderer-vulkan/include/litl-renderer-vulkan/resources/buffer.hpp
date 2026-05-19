#ifndef LITL_RENDERER_VULKAN_BUFFER_H__
#define LITL_RENDERER_VULKAN_BUFFER_H__

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/buffer.hpp"

namespace litl::vulkan
{
    struct BufferResource
    {
        VkBuffer vkBuffer = VK_NULL_HANDLE;
        VmaAllocator vmaAllocator = VK_NULL_HANDLE;

        // ... todo ...
    };
}

#endif