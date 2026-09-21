#ifndef LITL_MATH_TEXTURE_UTILS_H__
#define LITL_MATH_TEXTURE_UTILS_H__

#include <bit>
#include <cstdint>

#include "litl-core/math/common.hpp"
#include "litl-core/formats/dataFormats.hpp"

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

    /// <summary>
    /// Returns the total number of levels to represent all of the mipmaps for a texture with the given dimensions.
    /// </summary>
    [[nodiscard]] constexpr uint32_t mipLevelCount(uint32_t width, uint32_t height, uint32_t depth) noexcept
    {
        const uint32_t largest = max(width, max(height, max(depth, 1u)));
        return static_cast<uint32_t>(std::bit_width(largest));
    }

    /// <summary>
    /// Dimensions of a texel block for a given format.
    /// </summary>
    struct TexelBlockExtent
    {
        uint32_t width{ 1u };
        uint32_t height{ 1u };
    };

    /// <summary>
    /// The number of texels sampled at a time for each format.
    /// Uncompressed textures sample an individual texel (1x1 block), while compressed textures sample a 4x4 block of texels.
    /// </summary>
    [[nodiscard]] constexpr TexelBlockExtent texelBlockExtent(DataFormat format) noexcept
    {
        switch (format)
        {
            // Compressed formats are 4x4
        case DataFormat::BC4_UNorm:
        case DataFormat::BC5_UNorm:
        case DataFormat::BC6H_UFloat:
        case DataFormat::BC6H_SFloat:
        case DataFormat::BC7_UNorm:
        case DataFormat::BC7_SRGB:
            return { 4u, 4u };

            // Uncompressed formats are 1x1
        case DataFormat::RGBA8_UNorm:
        case DataFormat::RGBA8_SRGB:
        case DataFormat::BGRA8_Unorm:
        case DataFormat::BGRA8_SRGB:
        case DataFormat::ABGR10_UNorm_Pack32:
        case DataFormat::RGBA16_SFloat:
        case DataFormat::RGB32_SFloat:
        case DataFormat::RGBA32_SFloat:
        case DataFormat::R11G11B10_UFloat:
        case DataFormat::R8_UNorm:
        case DataFormat::R16_SFloat:
        case DataFormat::R32_SFloat:
        case DataFormat::RG8_UNorm:
        case DataFormat::RG16_SFloat:
        case DataFormat::RG32_SFloat:
        case DataFormat::D32_SFloat:
        case DataFormat::D24_UNorm_S8_UInt:
        case DataFormat::D32_SFloat_S8_UInt:
        case DataFormat::Undefined:
            return { 1u, 1u };
        }

        return { 1u, 1u };
    }

    /// <summary>
    /// Size, in bytes, of a single tightly packed mip level of a color or block-compressed image.
    /// Dimensions are rounded up to whole texel blocks, a 1x1 BC7 mip tail still occupies a full 16-byte block.
    /// Not valid for depth/stencil formats as the depth and stencil components are treated as two distinct regions.
    /// </summary>
    [[nodiscard]] constexpr uint64_t imageLevelBytes(DataFormat format, uint32_t width, uint32_t height, uint32_t depth) noexcept
    {
        if (dataFormatHasDepth(format) || dataFormatHasStencil(format))
        {
            return 0ull;
        }

        const TexelBlockExtent block = texelBlockExtent(format);

        const uint64_t blocksX = (static_cast<uint64_t>(width) + block.width - 1u) / block.width;
        const uint64_t blocksY = (static_cast<uint64_t>(height) + block.height - 1u) / block.height;

        return blocksX * blocksY * static_cast<uint64_t>(depth) * static_cast<uint64_t>(dataFormatSize(format));
    }

    /// <summary>
    /// Given the original extents of an image, calculates the total bytes needed to store it and all of its mipmaps.
    /// </summary>
    [[nodiscard]] constexpr uint64_t imageChainBytes(DataFormat format, uint32_t width, uint32_t height, uint32_t depth) noexcept
    {
        const uint32_t mipLevels = mipLevelCount(width, height, depth);
        uint64_t totalBytes = 0ull;

        for (uint32_t i = 0u; i < mipLevels; ++i)
        {
            totalBytes += imageLevelBytes(format, mipExtent(width, i), mipExtent(height, i), mipExtent(depth, i));
        }

        return totalBytes;
    }
}

#endif