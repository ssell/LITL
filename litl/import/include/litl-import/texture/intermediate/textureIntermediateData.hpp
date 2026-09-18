#ifndef LITL_IMPORT_TEXTURE_INTERMEDIATE_DATA_H__
#define LITL_IMPORT_TEXTURE_INTERMEDIATE_DATA_H__

#include <cstdint>
#include <span>
#include <vector>

#include "litl-core/formats/dataFormats.hpp"

namespace litl::import
{
    /// <summary>
    /// How the texture is to be interpreted.
    /// </summary>
    enum class TextureSemantic
    {
        Unknown       = 0u,
        Albedo        = 1u,     // Color data (RGBA8_SRGB)
        NormalTangent = 2u,     // Tangent-space normal map (RGBA8_UNORM)
        Mask          = 3u,     // Masking texture (RGBA8_UNORM)
        Hdr           = 4u      // High-dynamic range texture (RGBA16_SFLOAT)
    };

    struct TextureLevel
    {
        uint64_t byteOffset{ 0ull };
        uint64_t byteSize{ 0ull };
        uint32_t width{ 1u };
        uint32_t height{ 1u };
        uint32_t depth{ 1u };
    };

    struct TextureDataDescriptor
    {
        DataFormat format{ DataFormat::Undefined };
        uint32_t width{ 1u };
        uint32_t height{ 1u };
        uint32_t depth{ 1u };
        uint32_t arrayLayers{ 1u };
        TextureSemantic semantic{ TextureSemantic::Unknown };
        bool isCubeMap{ false };
        bool alphaPremultiplied{ false };
    };

    /// <summary>
    /// Intermediate representation of a texture. All external texture formats converge onto this shape.
    /// It should be noted that all formats are expanded to RGBA for both consistency and hardware support
    /// as RGB formats (for example VK_FORMAT_R8G8B8_*) are frequently unsupported as sampled images.
    /// 
    /// Additionally all pixel data should be top-left origin orientated.
    /// </summary>
    class TextureIntermediateData
    {
    public:

        TextureDataDescriptor& getDataDescriptorWriteRef() noexcept;
        TextureDataDescriptor const& getDataDescriptor() const noexcept;

        [[nodiscard]] std::vector<TextureLevel>& getTextureLevelsWriteRef() noexcept;
        [[nodiscard]] std::span<TextureLevel const> getTextureLevels() const noexcept;

        [[nodiscard]] std::vector<std::byte>& getPixelBytesWriteRef() noexcept;
        [[nodiscard]] std::span<std::byte const> getPixelBytes() const noexcept;

        [[nodiscard]] bool validate() const noexcept;

    private:

        TextureDataDescriptor m_dataDescriptor{};
        std::vector<TextureLevel> m_levels;             // m_levels[0] is always present. Mipmap generation appends additional levels.
        std::vector<std::byte> m_pixels;                // raw pixel data. m_levels indexes into this.
    };
}

#endif