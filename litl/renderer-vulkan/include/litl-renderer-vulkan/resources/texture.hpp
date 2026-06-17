#ifndef LITL_RENDERER_VULKAN_TEXTURE_H__
#define LITL_RENDERER_VULKAN_TEXTURE_H__

#include "litl-core/stringId.hpp"
#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/texture.hpp"

namespace litl::vulkan
{
    struct TextureResource
    {
        /// <summary>
        /// The Vulkan image handle.
        /// </summary>
        VkImage vkImage = VK_NULL_HANDLE;

        /// <summary>
        /// The texture format. 
        /// Duplicated here (also available in the descriptor) to reduce runtime DataFormat -> vkFormat conversions.
        /// </summary>
        VkFormat vkFormat = VkFormat::VK_FORMAT_UNDEFINED;

        /// <summary>
        /// Format + Aspect + Mip Range + Array Range
        /// </summary>
        VkImageView vkImageView = VK_NULL_HANDLE;

        /// <summary>
        /// The VMA allocated texture memory.
        /// </summary>
        VmaAllocation allocation = VK_NULL_HANDLE;

        /// <summary>
        /// If the texture was provided a name, this is the hashed name.
        /// </summary>
        StringId id{};

        /// <summary>
        /// The descriptor that created the texture.
        /// </summary>
        TextureDescriptor descriptor{};
    };
}

#endif