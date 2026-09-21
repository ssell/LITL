#ifndef LITL_CORE_FORMATS_SRGB_H__
#define LITL_CORE_FORMATS_SRGB_H__

#include "litl-core/math/types/color.hpp"

namespace litl
{
    enum class TransferFunction : uint8_t
    {
        /// <summary>
        /// Pixels are linear on the range (0-255) (0.0-1.0).
        /// Use for non-color textures such as normal maps, masks, roughness, etc.
        /// </summary>
        Linear = 0,

        /// <summary>
        /// Pixels are stored in sRGB gamma space which interpolates values more
        /// closely to how human eyes perceive them. For example, we see dark
        /// shades in more detail than bright ones.
        /// 
        /// Use for color textures such as albedo.
        /// 
        /// Most standard image formats such as JPEG, PNG, etc. store in sRGB.
        /// </summary>
        SRGB = 1
    };

    /// <summary>
    /// Returns the pre-calculted transfer table for converting a byte value to a linear float value.
    /// </summary>
    [[nodiscard]] std::array<float, 256> const& getByteToLinearFloatTable() noexcept;

    /// <summary>
    /// Returns the pre-calculted transfer table for converting a byte value to a SRGB float value.
    /// </summary>
    [[nodiscard]] std::array<float, 256> const& getByteToSRGBFloatTable() noexcept;

    /// <summary>
    /// Given a transfer function, returns a pre-calculated transfer table for all values of a uint8_t.
    /// </summary>
    [[nodiscard]] std::array<float, 256> const& getByteToFloatTable(TransferFunction function) noexcept;

    /// <summary>
    /// Converts a single linear component to sRGB gamma space using the true EOTF transfer function. 
    /// See: https://en.wikipedia.org/wiki/SRGB
    /// </summary>
    [[nodiscard]] float linearFloatToSRGBFloat(float c) noexcept;

    /// <summary>
    /// Converts a single sRGB gamma encoded float to a linear float.
    /// </summary>
    [[nodiscard]] float sRGBFloatToLinearFloat(float c);

    /// <summary>
    /// Converts a four-component linear color to sRGB color. Alpha is untouched.
    /// </summary>
    [[nodiscard]] color linearColorToSRGBColor(color c) noexcept;
}

#endif