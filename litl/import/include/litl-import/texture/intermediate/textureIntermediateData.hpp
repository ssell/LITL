#ifndef LITL_IMPORT_TEXTURE_INTERMEDIATE_DATA_H__
#define LITL_IMPORT_TEXTURE_INTERMEDIATE_DATA_H__

#include <cstdint>
#include <span>
#include <vector>

#include "litl-core/formats/dataFormats.hpp"
#include "litl-core/formats/srgb.hpp"

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
    /// An individual level/mipmap of the texture.
    /// </summary>
    struct TextureLevel
    {
        /// <summary>
        /// The offset into the pixels array where this level (mipmap) begins.
        /// </summary>
        uint64_t byteOffset{ 0ull };

        /// <summary>
        /// The total size of this level (mipmap) in bytes.
        /// </summary>
        uint64_t byteSize{ 0ull };

        /// <summary>
        /// Width of the level, in pixels.
        /// </summary>
        uint32_t width{ 1u };

        /// <summary>
        /// Height of the level, in pixels.
        /// </summary>
        uint32_t height{ 1u };

        /// <summary>
        /// The depth of the level, in pixels.
        /// </summary>
        uint32_t depth{ 1u };
    };

    struct TextureDataDescriptor
    {
        DataFormat format{ DataFormat::Undefined };
        uint32_t width{ 1u };
        uint32_t height{ 1u };
        uint32_t depth{ 1u };
        uint32_t arrayLayers{ 1u };
        uint32_t faceCount{ 1u };
        TransferFunction transfer{ TransferFunction::Linear };
        TextureSemantic semantic{ TextureSemantic::Unknown };
        bool isCubeMap{ false };
        bool alphaPremultiplied{ false };
        bool mipMaps{ false };
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
        /// 
        /// This will also populate the texture levels and size the pixel buffer accordingly if mipmaps are enabled in the data descriptor.
        /// However, it will not generate mipmaps and pixel bytes beyond the end of the first level (index 0) will be left at 0. The user
        /// is responsible for calling generateMipMaps to populate the rest of the pixel buffer.
        /// </summary>
        [[nodiscard]] bool store8BitPixelsAsFloat(std::span<std::byte const> pixelBytes) noexcept;

        /// <summary>
        /// Returns true if the descriptor and pixels are valid.
        /// </summary>
        [[nodiscard]] bool validate() const noexcept;

        /// <summary>
        /// Returns the number of levels in the texture.
        /// There is always at minimum 1 level. Mipmap generation increases the level count.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] uint32_t levelsCount() const noexcept;

        /// <summary>
        /// Generates mipmaps from the first level of pixel data.
        /// Expects the internal levels and pixels arrays to already be sized/ready to receive the mipmap data.
        /// If mipmaps are disabled in the texture descriptor then no action is performed and true is returned.
        /// </summary>
        [[nodiscard]] bool generateMipMaps() noexcept;

    private:

        TextureDataDescriptor m_dataDescriptor{};

        /// <summary>
        /// Array of levels/mipmaps in the texture.
        /// The first level (index 0) is always present. Mipmap generation appends additional levels.
        /// </summary>
        std::vector<TextureLevel> m_levels;

        /// <summary>
        /// The raw byte array of the texture and all of its mipmaps. Each pixel is expanded to 4 components: RGBA.
        /// The TextureLevel::byteOffset value indexes into here.
        /// </summary>
        std::vector<std::byte> m_pixels;
    };
}

#endif