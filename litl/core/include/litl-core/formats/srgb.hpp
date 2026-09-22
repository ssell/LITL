#ifndef LITL_CORE_FORMATS_SRGB_H__
#define LITL_CORE_FORMATS_SRGB_H__

#include <array>

#include "litl-core/math/types/color.hpp"
#include "litl-core/formats/transferFunction.hpp"

namespace litl
{
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