#ifndef LITL_RENDERER_VULKAN_TEXTURE_TABLE_H__
#define LITL_RENDERER_VULKAN_TEXTURE_TABLE_H__

#include <cstdint>
#include <vector>

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/texture.hpp"

namespace litl::vulkan
{
    struct RendererContext;
    struct TextureResource;

    /// <summary>
    /// The global texture table used for bindless texture sampling. Manages three distinct texture arrays:
    /// 
    ///     * g_Textures: 2D Textures bound at set 0, binding 0.
    ///     * g_TextureArrays: 2D Texture Arrays bound at set 0, binding 1.
    ///     * g_TextureCubeArrays: Cubemap Textures bound at set 0, binding 2.
    /// 
    /// When acquiring a texture slot index, the index is distinct based on the underlying array that is
    /// appropriate for the texture type. This means there may be duplicate integer indices handed out by
    /// the texture table, but there is no overlap of indices within the same type. Example:
    /// 
    ///     acquire(handle, false, false)   ->  0
    ///     acquire(handle, true, false)    ->  0
    ///     acquire(handle, false, true)    ->  0
    ///     acquire(handle, false, false)   ->  1
    ///     acquire(handle, false, false)   ->  2
    ///     acquire(handle, true, false)    ->  1
    /// </summary>
    class TextureTable final
    {
    public:

        [[nodiscard]] bool build(RendererContext& context) noexcept;
        void destroy() noexcept;

        [[nodiscard]] uint32_t acquire(TextureResourceHandle handle, bool isArray, bool isCube) noexcept;
        [[nodiscard]] bool release(uint32_t slot, TextureResourceHandle handle, bool isArray, bool isCube) noexcept;
        [[nodiscard]] bool update(uint32_t slot, TextureResourceHandle handle, bool isArray, bool isCube) noexcept;

        [[nodiscard]] VkDescriptorSet getDescriptorSet() const noexcept;
        [[nodiscard]] VkDescriptorSetLayout getDescriptorSetLayout() const noexcept;
        [[nodiscard]] VkPipelineLayout getPipelineLayout() const noexcept;

    private:

        enum class TextureSubTableType
        {
            Texture2D = 0u,
            Texture2DArray = 1u,
            Texture2DCubeArray = 2u,
            Count
        };

        struct TextureSubTable
        {
            std::vector<TextureResourceHandle> slotOwners;
            std::vector<uint32_t> freeSlots;
            uint32_t capacity{ 0u };
            uint32_t head{ 0u };
            uint32_t freeCount{ 0u };
            TextureResourceHandle fallbackHandle{};
        };

        [[nodiscard]] bool buildDescriptorPool() noexcept;
        [[nodiscard]] bool buildDescriptorSetLayout() noexcept;
        [[nodiscard]] bool buildDescriptorSet() noexcept;
        [[nodiscard]] bool buildPipelineLayout() noexcept;
        [[nodiscard]] bool writeSlot(uint32_t slot, TextureResource* texture) noexcept;
        [[nodiscard]] TextureSubTableType getSubTableType(bool isArray, bool isCube) const noexcept;
        [[nodiscard]] TextureSubTable& getSubTable(TextureSubTableType type) noexcept;

        RendererContext* m_pContext{ nullptr };
        VkDescriptorPool m_vkDescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSetLayout m_vkDescriptorSetLayout{ VK_NULL_HANDLE };
        VkDescriptorSet m_vkDescriptorSet{ VK_NULL_HANDLE };
        VkPipelineLayout m_vkPipelineLayout{ VK_NULL_HANDLE };
        std::array<TextureSubTable, static_cast<size_t>(TextureSubTableType::Count)> m_subtables;
    };
}

#endif