#ifndef LITL_RENDERER_VULKAN_COMMAND_BUFFER_H__
#define LITL_RENDERER_VULKAN_COMMAND_BUFFER_H__

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/commandBuffer.hpp"

namespace litl::vulkan
{
    struct CommandBufferResource
    {
        VkCommandBuffer vkCommandBuffer = VK_NULL_HANDLE;
        // ... todo ...
    };
}

#endif