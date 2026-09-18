#include "litl-core/formats/dataFormats.hpp"

namespace litl::tests
{
    // -------------------------------------------------------------------------------------
    // Static Compile-Time Tests
    // -------------------------------------------------------------------------------------

    static_assert(dataFormatHasDepth(DataFormat::D32_SFloat) == true);
    static_assert(dataFormatHasDepth(DataFormat::D24_UNorm_S8_UInt) == true);
    static_assert(dataFormatHasDepth(DataFormat::D32_SFloat_S8_UInt) == true);
    static_assert(dataFormatHasDepth(DataFormat::RGBA8_UNorm) == false);
    static_assert(dataFormatHasDepth(DataFormat::RGBA8_SRGB) == false);
    static_assert(dataFormatHasDepth(DataFormat::BGRA8_Unorm) == false);
    static_assert(dataFormatHasDepth(DataFormat::BGRA8_SRGB) == false);
    static_assert(dataFormatHasDepth(DataFormat::ABGR10_UNorm_Pack32) == false);
    static_assert(dataFormatHasDepth(DataFormat::RGBA16_SFloat) == false);
    static_assert(dataFormatHasDepth(DataFormat::RGB32_SFloat) == false);
    static_assert(dataFormatHasDepth(DataFormat::RGBA32_SFloat) == false);
    static_assert(dataFormatHasDepth(DataFormat::R11G11B10_UFloat) == false);
    static_assert(dataFormatHasDepth(DataFormat::R8_UNorm) == false);
    static_assert(dataFormatHasDepth(DataFormat::R16_SFloat) == false);
    static_assert(dataFormatHasDepth(DataFormat::R32_SFloat) == false);
    static_assert(dataFormatHasDepth(DataFormat::RG8_UNorm) == false);
    static_assert(dataFormatHasDepth(DataFormat::RG16_SFloat) == false);
    static_assert(dataFormatHasDepth(DataFormat::RG32_SFloat) == false);
    static_assert(dataFormatHasDepth(DataFormat::BC7_UNorm) == false);
    static_assert(dataFormatHasDepth(DataFormat::BC7_SRGB) == false);
    static_assert(dataFormatHasDepth(DataFormat::BC4_UNorm) == false);
    static_assert(dataFormatHasDepth(DataFormat::BC5_UNorm) == false);
    static_assert(dataFormatHasDepth(DataFormat::BC6H_UFloat) == false);
    static_assert(dataFormatHasDepth(DataFormat::BC6H_SFloat) == false);
    static_assert(dataFormatHasDepth(DataFormat::Undefined) == false);

    static_assert(dataFormatHasStencil(DataFormat::D24_UNorm_S8_UInt) == true);
    static_assert(dataFormatHasStencil(DataFormat::D32_SFloat_S8_UInt) == true);
    static_assert(dataFormatHasStencil(DataFormat::RGBA8_UNorm) == false);
    static_assert(dataFormatHasStencil(DataFormat::RGBA8_SRGB) == false);
    static_assert(dataFormatHasStencil(DataFormat::BGRA8_Unorm) == false);
    static_assert(dataFormatHasStencil(DataFormat::BGRA8_SRGB) == false);
    static_assert(dataFormatHasStencil(DataFormat::ABGR10_UNorm_Pack32) == false);
    static_assert(dataFormatHasStencil(DataFormat::RGBA16_SFloat) == false);
    static_assert(dataFormatHasStencil(DataFormat::RGB32_SFloat) == false);
    static_assert(dataFormatHasStencil(DataFormat::RGBA32_SFloat) == false);
    static_assert(dataFormatHasStencil(DataFormat::R11G11B10_UFloat) == false);
    static_assert(dataFormatHasStencil(DataFormat::R8_UNorm) == false);
    static_assert(dataFormatHasStencil(DataFormat::R16_SFloat) == false);
    static_assert(dataFormatHasStencil(DataFormat::R32_SFloat) == false);
    static_assert(dataFormatHasStencil(DataFormat::RG8_UNorm) == false);
    static_assert(dataFormatHasStencil(DataFormat::RG16_SFloat) == false);
    static_assert(dataFormatHasStencil(DataFormat::RG32_SFloat) == false);
    static_assert(dataFormatHasStencil(DataFormat::D32_SFloat) == false);
    static_assert(dataFormatHasStencil(DataFormat::BC7_UNorm) == false);
    static_assert(dataFormatHasStencil(DataFormat::BC7_SRGB) == false);
    static_assert(dataFormatHasStencil(DataFormat::BC4_UNorm) == false);
    static_assert(dataFormatHasStencil(DataFormat::BC5_UNorm) == false);
    static_assert(dataFormatHasStencil(DataFormat::BC6H_UFloat) == false);
    static_assert(dataFormatHasStencil(DataFormat::BC6H_SFloat) == false);
    static_assert(dataFormatHasStencil(DataFormat::Undefined) == false);

    static_assert(dataFormatSize(DataFormat::RGBA8_UNorm) == 4u);
    static_assert(dataFormatSize(DataFormat::RGBA8_SRGB) == 4u);
    static_assert(dataFormatSize(DataFormat::BGRA8_Unorm) == 4u);
    static_assert(dataFormatSize(DataFormat::BGRA8_SRGB) == 4u);
    static_assert(dataFormatSize(DataFormat::ABGR10_UNorm_Pack32) == 4u);
    static_assert(dataFormatSize(DataFormat::RGBA16_SFloat) == 8u);
    static_assert(dataFormatSize(DataFormat::RGB32_SFloat) == 12u);
    static_assert(dataFormatSize(DataFormat::RGBA32_SFloat) == 16u);
    static_assert(dataFormatSize(DataFormat::R11G11B10_UFloat) == 4u);
    static_assert(dataFormatSize(DataFormat::R8_UNorm) == 1u);
    static_assert(dataFormatSize(DataFormat::R16_SFloat) == 2u);
    static_assert(dataFormatSize(DataFormat::R32_SFloat) == 4u);
    static_assert(dataFormatSize(DataFormat::RG8_UNorm) == 2u);
    static_assert(dataFormatSize(DataFormat::RG16_SFloat) == 4u);
    static_assert(dataFormatSize(DataFormat::RG32_SFloat) == 8u);
    static_assert(dataFormatSize(DataFormat::D32_SFloat) == 4u);
    static_assert(dataFormatSize(DataFormat::D24_UNorm_S8_UInt) == 4u);
    static_assert(dataFormatSize(DataFormat::D32_SFloat_S8_UInt) == 8u);
    static_assert(dataFormatSize(DataFormat::BC7_UNorm) == 16u);
    static_assert(dataFormatSize(DataFormat::BC7_SRGB) == 16u);
    static_assert(dataFormatSize(DataFormat::BC4_UNorm) == 8u);
    static_assert(dataFormatSize(DataFormat::BC5_UNorm) == 16u);
    static_assert(dataFormatSize(DataFormat::BC6H_UFloat) == 16u);
    static_assert(dataFormatSize(DataFormat::BC6H_SFloat) == 16u);
    static_assert(dataFormatSize(DataFormat::Undefined) == 0u);
}