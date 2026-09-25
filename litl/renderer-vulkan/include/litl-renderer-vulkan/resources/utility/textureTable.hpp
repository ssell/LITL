#ifndef LITL_RENDERER_VULKAN_TEXTURE_TABLE_H__
#define LITL_RENDERER_VULKAN_TEXTURE_TABLE_H__

#include <cstdint>
#include <vector>

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/texture.hpp"

namespace litl::vulkan
{
    struct RendererContext;

    /// <summary>
    /// The global texture table used for bindless texture sampling.
    /// </summary>
    class TextureTable final
    {
    public:

        [[nodiscard]] bool build(RendererContext& context) noexcept;
        void destroy() noexcept;

        [[nodiscard]] uint32_t acquire(TextureResourceHandle handle) noexcept;
        [[nodiscard]] bool release(uint32_t slot) noexcept;
        [[nodiscard]] bool update(uint32_t slot, TextureResourceHandle handle) noexcept;

        [[nodiscard]] VkDescriptorSet getDescriptorSet() const noexcept;
        [[nodiscard]] VkDescriptorSetLayout getDescriptorSetLayout() const noexcept;

    private:

        [[nodiscard]] bool buildDescriptorPool() noexcept;
        [[nodiscard]] bool buildDescriptorSetLayout() noexcept;
        [[nodiscard]] bool buildDescriptorSet() noexcept;

        RendererContext* m_pContext{ nullptr };
        VkDescriptorPool m_vkDescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSetLayout m_vkDescriptorSetLayout{ VK_NULL_HANDLE };
        VkDescriptorSet m_vkDescriptorSet{ VK_NULL_HANDLE };
        std::vector<TextureResourceHandle> m_slotOwners;
        std::vector<uint32_t> m_freeSlots;
        uint32_t m_head{ 0u };
        uint32_t m_capacity{ 0u };
    };
}

#endif