#ifndef LITL_IMPORT_TEXTURE_SETTINGS_H__
#define LITL_IMPORT_TEXTURE_SETTINGS_H__

#include <cstdint>
#include <type_traits>

#include "litl-core/formats/transferFunction.hpp"
#include "litl-renderer/resources/textureChannel.hpp"

namespace litl::import
{
    /// <summary>
    /// How the texture is to be interpreted.
    /// </summary>
    enum class TextureSemantic : uint8_t
    {
        Unknown = 0u,
        BaseColor = 1u,         // SRGB color data (RGBA32_SFloat)
        LinearColor = 2u,       // Linear color data (RGBA32_SFloat)
        NormalTangent = 3u,     // Tangent-space normal map (RGBA32_SFloat)
        Mask = 4u,              // Masking texture (RGBA32_SFloat)
        Hdr = 5u                // High-dynamic range texture (RGBA16_SFLOAT)
    };

    struct TextureImportSettings
    {
        TextureSemantic semantic{ TextureSemantic::Unknown };
        TransferFunction transfer{ TransferFunction::Linear };
        TextureChannel channel{ TextureChannel::All };
        bool mipmaps{ false };
        bool residesInTextureTable{ true };
    };

    /// <summary>
    /// Base color/albedo maps.
    /// </summary>
    inline constexpr TextureImportSettings ColorTextureImportSettings{
        .semantic = TextureSemantic::BaseColor,
        .transfer = TransferFunction::SRGB,
        .mipmaps = true
    };

    /// <summary>
    /// RGBA color maps that are not SRGB but want mipmaps. Metallic, roughness, etc.
    /// </summary>
    inline constexpr TextureImportSettings LinearColorTextureImportSettings{
        .semantic = TextureSemantic::LinearColor,
        .transfer = TransferFunction::Linear,
        .mipmaps = true
    };

    /// <summary>
    /// Normal tangent maps.
    /// </summary>
    inline constexpr TextureImportSettings NormalMapImportSettings{
        .semantic = TextureSemantic::NormalTangent,
        .transfer = TransferFunction::Linear,
        .mipmaps = true
    };

    /// <summary>
    /// Pure data textures without mipmaps.
    /// </summary>
    inline constexpr TextureImportSettings MaskTextureImportSettings{
        .semantic = TextureSemantic::Mask,
        .transfer = TransferFunction::Linear,
        .mipmaps = false
    };

    static_assert(std::is_trivially_copyable_v<TextureImportSettings>);
}

#endif