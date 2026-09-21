#ifndef LITL_MATH_TEXTURE_UTILS_H__
#define LITL_MATH_TEXTURE_UTILS_H__

#include <cstdint>

namespace litl
{
    /// <summary>
    /// The extent of the given mip level, derived from the base extent. Never returns 0.
    /// 
    /// An extent defines the exact pixel dimension (width, height, and depth) of a specific 
    /// downsampled image in a mipmap chain. The base level (0) represents the original, highest-resolution 
    /// texture dimensions. Each subsequent mip level (level + 1) custs the previous levels width, height, 
    /// and depth dimensions in half, rounding down until it reaches 1 pixel.
    /// </summary>
    [[nodiscard]] constexpr uint32_t mipExtent(uint32_t baseExtent, uint32_t level) noexcept
    {
        return (level >= 32u) ? 1u : ((baseExtent >> level) > 0u ? (baseExtent >> level) : 1u);
    }
}

#endif