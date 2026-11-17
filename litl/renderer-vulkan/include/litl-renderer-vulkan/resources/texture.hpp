#ifndef LITL_RENDERER_VULKAN_TEXTURE_H__
#define LITL_RENDERER_VULKAN_TEXTURE_H__

#include <vector>

#include "litl-core/assert.hpp"
#include "litl-core/stringId.hpp"
#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/texture.hpp"

namespace litl::vulkan
{
    struct TextureMemoryMap
    {
        /// <summary>
        /// 
        /// </summary>
        void* persistent = nullptr;

        /// <summary>
        /// 
        /// </summary>
        void* temporary = nullptr;
    };

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
        /// For use with textures that require a separate view for sampling (such as depth-stencil).
        /// </summary>
        VkImageView vkSampledImageView = VK_NULL_HANDLE;

        /// <summary>
        /// If the texture has more than one layer, and the texture has a color or depth attachment, these are
        /// the per-layer attachment views. Needed to dynamically render into a specific layer (shadow cascades, etc.).
        /// </summary>
        std::vector<VkImageView> layerAttachmentViews;

        /// <summary>
        /// 
        /// </summary>
        VkExtent3D vkExtent{};

        /// <summary>
        /// Used by view creation, barriers, transitions, etc.
        /// </summary>
        VkImageSubresourceRange vkImageSubresourceRange{};

        /// <summary>
        /// The VMA allocated texture memory.
        /// </summary>
        VmaAllocation allocation = VK_NULL_HANDLE;

        /// <summary>
        /// Optional pointer to mapped memory, offset, etc.
        /// </summary>
        VmaAllocationInfo allocationInfo{};

        /// <summary>
        /// 
        /// </summary>
        TextureMemoryMap memoryMap{};

        /// <summary>
        /// If the texture was provided a name, this is the hashed name.
        /// </summary>
        StringId id{};

        /// <summary>
        /// The slot that this texture occupies in the global bindless texture table.
        /// </summary>
        uint32_t textureTableSlot{ constants::uint32_null_index };

        /// <summary>
        /// The descriptor that created the texture.
        /// </summary>
        TextureResourceDescriptor descriptor{};

        [[nodiscard]] VkImageView getSampledView() const noexcept
        {
            return (vkSampledImageView != VK_NULL_HANDLE) ? vkSampledImageView : vkImageView;
        }

        [[nodiscard]] VkImageView getAttachmentView(uint32_t layer) const noexcept
        {
            if (layerAttachmentViews.empty())
            {
                return vkImageView;
            }
            else
            {
                LITL_ASSERT_MSG((layer < layerAttachmentViews.size()), "TextureResource::getAttachmentView called with out-of-bounds layer index. Defaulting to index 0.", layerAttachmentViews[0]);
                return layerAttachmentViews[layer];
            }
        }
    };
}

#endif