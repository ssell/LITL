#ifndef LITL_RENDERER_UTILITY_H__
#define LITL_RENDERER_UTILITY_H__

#include <cstdint>

namespace litl
{
    /// <summary>
    /// Packs the texture slot and sampler index together into a single 32-bit unsigned integer.
    /// The texture slot comprises the lower 24-bits while the sampler index resides in the upper 8-bits.
    /// This gives room for 16.7M textures and 256 samplers.
    /// </summary>
    [[nodiscard]] constexpr uint32_t packTextureSlotSamplerIndex(uint32_t textureSlot, uint32_t samplerIndex) noexcept
    {
        return ((samplerIndex & 0xFFu) << 24u) | (textureSlot & 0x00FFFFFF);
    }

    /// <summary>
    /// Given a packed texture slot and sampler index, extracts the texture slot from the lower 24-bits.
    /// </summary>
    [[nodiscard]] constexpr uint32_t extractTextureSlot(uint32_t packedTextureSlotSamplerIndex) noexcept
    {
        return (packedTextureSlotSamplerIndex & 0x00FFFFFFu);
    }

    /// <summary>
    /// Given a packed texture slot and sampler index, extracts the sampler index from the upper 8-bits.
    /// </summary>
    [[nodiscard]] constexpr uint32_t extractSamplerIndex(uint32_t packedTextureSlotSamplerIndex) noexcept
    {
        return (packedTextureSlotSamplerIndex >> 24u);
    }
}

#endif