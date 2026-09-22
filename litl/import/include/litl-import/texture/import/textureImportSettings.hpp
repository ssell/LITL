#ifndef LITL_IMPORT_TEXTURE_SETTINGS_H__
#define LITL_IMPORT_TEXTURE_SETTINGS_H__

#include <cstdint>
#include <type_traits>

#include "litl-core/formats/transferFunction.hpp"

namespace litl::import
{
    /// <summary>
    /// How the texture is to be interpreted.
    /// </summary>
    enum class TextureSemantic : uint8_t
    {
        Unknown = 0u,
        Albedo = 1u,            // Color data (RGBA32_SFloat)
        NormalTangent = 2u,     // Tangent-space normal map (RGBA32_SFloat)
        Mask = 3u,              // Masking texture (RGBA32_SFloat)
        Hdr = 4u                // High-dynamic range texture (RGBA16_SFLOAT)
    };

    struct TextureImportSettings
    {
        TextureSemantic semantic{ TextureSemantic::Unknown };
        TransferFunction transfer{ TransferFunction::Linear };
        bool mipmaps{ false };
    };

    inline constexpr TextureImportSettings ColorTextureImportSettings{
        .semantic = TextureSemantic::Albedo,
        .transfer = TransferFunction::SRGB,
        .mipmaps = true
    };

    inline constexpr TextureImportSettings NormalMapImportSettings{
        .semantic = TextureSemantic::NormalTangent,
        .transfer = TransferFunction::Linear,
        .mipmaps = false
    };

    inline constexpr TextureImportSettings MaskTextureImportSettings{
        .semantic = TextureSemantic::Mask,
        .transfer = TransferFunction::Linear,
        .mipmaps = false
    };

    static_assert(std::is_trivially_copyable_v<TextureImportSettings>);
}

#endif