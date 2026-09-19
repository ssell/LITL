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
    enum class TextureSemantic : uint8_t
    {
        Unknown       = 0u,
        Albedo        = 1u,     // Color data (RGBA32_SFloat)
        NormalTangent = 2u,     // Tangent-space normal map (RGBA32_SFloat)
        Mask          = 3u,     // Masking texture (RGBA32_SFloat)
        Hdr           = 4u      // High-dynamic range texture (RGBA16_SFLOAT)
    };

    /// <summary>
    /// How the pixels are transferred.
    /// </summary>
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
        TransferFunction transfer{ TransferFunction::Linear };
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

        /// <summary>
        /// Given a span of four-component pixels stored as 8-bit components, converts and stores them as four-component float pixels.
        /// Note that if the descriptor has a transfer value of SRGB, then the incoming bytes will be converted to linear from sRGB gamma space.
        /// </summary>
        [[nodiscard]] bool store8BitPixelsAsFloat(std::span<uint8_t const> pixels) noexcept;

        /// <summary>
        /// Returns true if the descriptor and pixels are valid.
        /// </summary>
        [[nodiscard]] bool validate() const noexcept;

    private:

        TextureDataDescriptor m_dataDescriptor{};
        std::vector<TextureLevel> m_levels;             // m_levels[0] is always present. Mipmap generation appends additional levels.
        std::vector<std::byte> m_pixels;                // raw pixel data. m_levels indexes into this.
    };
}

#endif