#ifndef LITL_RENDERER_VULKAN_TEXTURE_TABLE_H__
#define LITL_RENDERER_VULKAN_TEXTURE_TABLE_H__

#include <cstdint>
#include <vector>

#include "litl-renderer-vulkan/common.hpp"

namespace litl::vulkan
{
    struct RendererContext;
    struct TextureResource;

    /// <summary>
    /// The global texture table used for bindless texture sampling.
    /// </summary>
    class TextureTable final
    {
    public:

        [[nodiscard]] bool build(RendererContext& context, uint32_t capacity) noexcept;
        void destroy() noexcept;

        [[nodiscard]] uint32_t acquire(TextureResource const& texture) noexcept;
        void release(uint32_t slot) noexcept;
        void update(uint32_t slot, TextureResource const& texture) noexcept;

        [[nodiscard]] VkDescriptorSet getSet() const noexcept;
        [[nodiscard]] VkDescriptorSetLayout getLayout() const noexcept;

    private:

        [[nodiscard]] bool buildDescriptorPool() noexcept;
        [[nodiscard]] bool buildDescriptorSetLayout() noexcept;
        [[nodiscard]] bool buildDescriptorSet() noexcept;

        VkDevice m_vkDevice{ VK_NULL_HANDLE };
        VkDescriptorPool m_vkDescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSetLayout m_vkDescriptorSetLayout{ VK_NULL_HANDLE };
        VkDescriptorSet m_vkDescriptorSet{ VK_NULL_HANDLE };
        std::vector<uint32_t> m_freeSlots;
        uint32_t m_head{ 0u };
        uint32_t m_capacity{ 0u };
    };
}

#endif