#ifndef LITL_RENDERER_TEXTURE_H__
#define LITL_RENDERER_TEXTURE_H__

#include <cstdint>
#include <string>

#include "litl-core/handles.hpp"
#include "litl-renderer/enums.hpp"

namespace litl
{
    struct TextureDescriptor
    {
        /// <summary>
        /// The dimensionality of the texture.
        /// </summary>
        TextureDimensions dimensions = TextureDimensions::Texture2D;

        /// <summary>
        /// Width of the texture, in pixels.
        /// </summary>
        uint32_t width = 1u;

        /// <summary>
        /// Height of the texture, in pixels.
        /// </summary>
        uint32_t height = 1u;

        /// <summary>
        /// Depth of the texture in pixels.
        /// </summary>
        uint32_t depth = 1u;

        /// <summary>
        /// Format of the image.
        /// </summary>
        DataFormat format = DataFormat::Undefined;

        /// <summary>
        /// How the texture will be used.
        /// </summary>
        TextureUsageFlag usage = TextureUsageFlagBits::TransferDest | TextureUsageFlagBits::Sampled;

        /// <summary>
        /// Where the memory for the buffer resides.
        /// </summary>
        BufferMemoryType memory = BufferMemoryType::Auto;

        /// <summary>
        /// How the memory will be accessed.
        /// </summary>
        BufferMemoryUsage memoryUsage = BufferMemoryUsage::GpuOnly;

        /// <summary>
        /// How the buffer can be shared between queue families.
        /// </summary>
        SharingMode sharing = SharingMode::Exclusive;

        /// <summary>
        /// The number of levels of detail available for minified sampling of the image.
        /// Set to 1 for a standard texture without any mipmaps.
        /// </summary>
        uint32_t mipLevels = 1u;

        /// <summary>
        /// If a texture array, the number of array layers.
        /// Set to 1 for a standard, single-layered texture.
        /// </summary>
        uint32_t arrayLayers = 1u;

        /// <summary>
        /// The number of samples per pixel. Primarily used for anti-aliasing.
        /// Set to a count of 1 for no multisampling.
        /// </summary>
        MultisampleCount sampleCount = MultisampleCount::Count1;

        /// <summary>
        /// Is the texture to be intrepeted as a cube map?
        /// </summary>
        bool isCubeMap = false;

        /// <summary>
        /// Optional name for the texture.
        /// If specified, it needs to be unique (or the original resource path).
        /// Used for hotreloads and debugging.
        /// </summary>
        std::string name;
    };

    struct TextureTag {};
    using TextureHandle = Handle<TextureTag>;

    struct MappedTexture
    {
        /// <summary>
        /// If the texture is mapped, this is the CPU address of the start of its memory block.
        /// </summary>
        void* mappedPtr = nullptr;
    };

    // -------------------------------------------------------------------------------------
    // Texture Utilities
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Dimensions of a texel block for a given format.
    /// </summary>
    struct TexelBlockExtent
    {
        uint32_t width{ 0u };
        uint32_t height{ 0u };
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
        const TexelBlockExtent block = texelBlockExtent(format);

        const uint64_t blocksX = (static_cast<uint64_t>(width) + block.width - 1u) / block.width;
        const uint64_t blocksY = (static_cast<uint64_t>(height) + block.height - 1u) / block.height;

        return blocksX * blocksY * static_cast<uint64_t>(depth) * static_cast<uint64_t>(dataFormatSize(format));
    }

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