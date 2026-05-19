#ifndef LITL_RENDERER_VULKAN_TEXTURE_H__
#define LITL_RENDERER_VULKAN_TEXTURE_H__

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/texture.hpp"

namespace litl::vulkan
{
    struct TextureResource
    {
        VkImage vkImage = VK_NULL_HANDLE;
        VmaAllocator vmaAllocator = VK_NULL_HANDLE;

        // ... todo ...
    };
}

#endif