#include "litl-core/logging/logging.hpp"
#include "litl-core/formats/srgb.hpp"

namespace litl
{
    std::array<float, 256> const& getByteToLinearFloatTable() noexcept
    {
        static const std::array<float, 256> byteToLinearFloatTable = []() {
            std::array<float, 256> table{};

            for (auto i = 0; i < 256; ++i)
            {
                table[i] = static_cast<float>(i) / 255.0f;
            }

            return table;
        }();

        return byteToLinearFloatTable;
    }

    std::array<float, 256> const& getByteToSRGBFloatTable() noexcept
    {
        static const std::array<float, 256> byteToSRGBFloatTable = []() {
            std::array<float, 256> table{};

            for (auto i = 0; i < 256; ++i)
            {
                table[i] = linearFloatToSRGBFloat(static_cast<float>(i) / 255.0f);
            }

            return table;
        }();

        return byteToSRGBFloatTable;
    }

    std::array<float, 256> const& getByteToFloatTable(TransferFunction function) noexcept
    {
        switch (function)
        {
        case TransferFunction::Linear:
            return getByteToLinearFloatTable();

        case TransferFunction::SRGB:
            return getByteToSRGBFloatTable();

        default:
            logError("Unknown transfer function supplied to byteToFloatTable. Falling back to linear table.");
            return getByteToLinearFloatTable();
        }
    }

    float linearFloatToSRGBFloat(float c) noexcept
    {
        static constexpr float one_over_12p92 = 1.0f / 12.92f;
        static constexpr float one_over1p055 = 1.0f / 1.055f;
        return (c < 0.04045f) ? (c * one_over_12p92) : powf((c + 0.055f) * one_over1p055, 2.4f);
    }

    float sRGBFloatToLinearFloat(float c)
    {
        static constexpr float one_over_2p4 = 1.0f / 2.4f;
        return (c <= 0.0031308) ? (c * 12.92f) : (1.055f * powf(c, one_over_2p4)) - 0.055f;
    }

    color linearColorToSRGBColor(color c) noexcept
    {
        return color{
            linearFloatToSRGBFloat(c.r()),
            linearFloatToSRGBFloat(c.g()),
            linearFloatToSRGBFloat(c.b()),
            c.a()                           // alpha stays as linear
        };
    }
}