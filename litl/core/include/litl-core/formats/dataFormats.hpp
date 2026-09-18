#ifndef LITL_CORE_DATA_FORMATS_H__
#define LITL_CORE_DATA_FORMATS_H__

#include <cstdint>
#include "litl-core/enumBitFlags.hpp"

namespace litl
{
    /// <summary>
    /// Different supported texture formats.
    /// 
    /// UNORM: the stored bytes are interpreted as linear values directly. 0-255 -> 0.0-1.0 linearly.
    /// SRGB: the stored bytes are interpreted as being in sRGB gamma space and the hardware automatically applies the sRGB->linear EOTF conversion during texel fetch, before the value reaches the shader as a float.
    /// 
    /// Typically UNORM for data textures that are not color textures such as normal maps, roughness/ao, heightmaps, velocity buffers, etc.
    /// Typically SRGB for externally authored color textures such as albedo/diffuse maps, UI textures, etc.
    /// 
    /// The most commonly used formats are the compressed formats. Used as:
    /// 
    /// BC7_SRGB: for high-quality color textures such as albedo, emissive, etc.
    /// BC6H_UFloat: for HDR content such as skyboxes, cubemap irradiance, baked lightmaps, etc.
    /// BC5_UNorm: for tangent-space normal maps. XY, reconstruct Z.
    /// BC4_UNorm: for grayscale masks, heightmaps, roughness/AO packed alone, etc.
    /// </summary>
    enum class DataFormat : uint32_t
    {
        Undefined = 0,

        // Color LDR
        RGBA8_UNorm,            // 32-bit RGBA color where each channel gets a 8-bit linear floating point value on the range [0,1].
        RGBA8_SRGB,             // 32-bit RGBA color where each channel gets a 8-bit non-linear (gamma corrected) float point values on the range [0,1].
        BGRA8_Unorm,            // 32-bit BGRA color where each channel gets a 8-bit linear floating point value on the range [0,1].
        BGRA8_SRGB,             // 32-bit BGRA color where each channel gets a 8-bit non-linear (gamma corrected) float point values on the range [0,1].
        ABGR10_UNorm_Pack32,    // 10-bit color, sometimes preferred for HDR-ish output or higher-precision G-buffers.

        // HDR
        RGBA16_SFloat,          // 64-bit RGBA color where each channel gets a 16-bit floating point value.
        RGB32_SFloat,           // 96-bit RGB color where each channel gets a 32-bit floating point value. Note this is typically used a data format (for example in vertex input layouts) and is not widely supported as a texture format.
        RGBA32_SFloat,          // 128-bit RGBA color where each channel gets a 32-bit floating point value.
        R11G11B10_UFloat,       // 32-bit RGB color where R and G gets a 11-bit and B gets a 10-bit unsigned floating point value.

        // Single Channel
        R8_UNorm,               // Single 8-bit linear red channel on the range [0,1].
        R16_SFloat,             // Single 16-bit floating point value in the red channel.
        R32_SFloat,             // Single 32-bit floating point value in the red channel.

        // Dual Channel
        RG8_UNorm,              // Double channel 8-bit red and green on the range [0,1].
        RG16_SFloat,            // Double channel 16-bit red and green floating point values.
        RG32_SFloat,            // Double channel 32-bit.

        // Depth
        D32_SFloat,             // 32-bit depth floating point value.
        D24_UNorm_S8_UInt,      // 32-bit format with a 24-bit linear depth value on the range [0,1] and 8-bit unsigned integer stencil.
        D32_SFloat_S8_UInt,     // 32-bit format with a 32-bit floating point depth value and 8-bit unsigned integer stencil.

        // Compressed
        BC4_UNorm,              // A one-component, block-compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized red texel data.
        BC5_UNorm,              // A two-component, block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RG texel data with the first 64 bits encoding red values followed by 64 bits encoding green values.
        BC6H_UFloat,            // A three-component, block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned floating-point RGB texel data. No alpha, strictly RGB.
        BC6H_SFloat,            // A three-component, block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of signed floating-point RGB texel data. No alpha, strictly RGB.
        BC7_UNorm,              // A four-component, block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data. Used for color textures such as albedo, emissive, etc.
        BC7_SRGB,               // A four-component, block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components. Used for color textures such as albedo, emissive, etc.
    };

    [[nodiscard]] constexpr bool dataFormatHasDepth(DataFormat format) noexcept
    {
        switch (format)
        {
        case DataFormat::D32_SFloat:
        case DataFormat::D24_UNorm_S8_UInt:
        case DataFormat::D32_SFloat_S8_UInt:
            return true;

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
        case DataFormat::BC7_UNorm:
        case DataFormat::BC7_SRGB:
        case DataFormat::BC4_UNorm:
        case DataFormat::BC5_UNorm:
        case DataFormat::BC6H_UFloat:
        case DataFormat::BC6H_SFloat:
        case DataFormat::Undefined:
            return false;
        }

        return false;
    }

    [[nodiscard]] constexpr bool dataFormatHasStencil(DataFormat format) noexcept
    {
        switch (format)
        {
        case DataFormat::D24_UNorm_S8_UInt:
        case DataFormat::D32_SFloat_S8_UInt:
            return true;

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
        case DataFormat::BC7_UNorm:
        case DataFormat::BC7_SRGB:
        case DataFormat::BC4_UNorm:
        case DataFormat::BC5_UNorm:
        case DataFormat::BC6H_UFloat:
        case DataFormat::BC6H_SFloat:
        case DataFormat::Undefined:
            return false;
        }

        return false;
    }

    /// <summary>
    /// Returns the number of bytes per each texel block for a given format.
    /// </summary>
    [[nodiscard]] constexpr uint32_t dataFormatSize(DataFormat format) noexcept
    {
        switch (format)
        {
        case DataFormat::RGBA8_UNorm: return 4u;
        case DataFormat::RGBA8_SRGB: return 4u;
        case DataFormat::BGRA8_Unorm: return 4u;
        case DataFormat::BGRA8_SRGB: return 4u;
        case DataFormat::ABGR10_UNorm_Pack32: return 4u;
        case DataFormat::RGBA16_SFloat: return 8u;
        case DataFormat::RGB32_SFloat: return 12u;
        case DataFormat::RGBA32_SFloat: return 16u;
        case DataFormat::R11G11B10_UFloat: return 4u;
        case DataFormat::R8_UNorm: return 1u;
        case DataFormat::R16_SFloat: return 2u;
        case DataFormat::R32_SFloat: return 4u;
        case DataFormat::RG8_UNorm: return 2u;
        case DataFormat::RG16_SFloat: return 4u;
        case DataFormat::RG32_SFloat: return 8u;
        case DataFormat::D32_SFloat: return 4u;
        case DataFormat::D24_UNorm_S8_UInt: return 4u;
        case DataFormat::D32_SFloat_S8_UInt: return 8u;
        case DataFormat::BC7_UNorm: return 16u;
        case DataFormat::BC7_SRGB: return 16u;
        case DataFormat::BC4_UNorm: return 8u;
        case DataFormat::BC5_UNorm: return 16u;
        case DataFormat::BC6H_UFloat: return 16u;
        case DataFormat::BC6H_SFloat: return 16u;
        case DataFormat::Undefined: return 0u;
        }

        return 0u;
    }
}

#endif