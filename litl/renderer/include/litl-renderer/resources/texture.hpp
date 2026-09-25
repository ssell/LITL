#ifndef LITL_RENDERER_TEXTURE_H__
#define LITL_RENDERER_TEXTURE_H__

#include <cstdint>
#include <string>
#include <vector>

#include "litl-core/handles.hpp"
#include "litl-core/formats/dataFormats.hpp"
#include "litl-core/math/textureUtils.hpp"
#include "litl-renderer/enums.hpp"

namespace litl
{
    struct TextureResourceDescriptor
    {
        /// <summary>
        /// The dimensionality of the texture.
        /// </summary>
        TextureDimensions dimensions = TextureDimensions::Texture2D;

        /// <summary>
        /// Width of the texture, in pixels.
        /// </summary>
        uint32_t width = 1u;

        /// <summary>
        /// Height of the texture, in pixels.
        /// </summary>
        uint32_t height = 1u;

        /// <summary>
        /// Depth of the texture in pixels.
        /// </summary>
        uint32_t depth = 1u;

        /// <summary>
        /// Format of the image.
        /// </summary>
        DataFormat format = DataFormat::Undefined;

        /// <summary>
        /// How the texture will be used.
        /// </summary>
        TextureUsageFlag usage = TextureUsageFlagBits::TransferDest | TextureUsageFlagBits::Sampled;

        /// <summary>
        /// Where the memory for the buffer resides.
        /// </summary>
        BufferMemoryType memory = BufferMemoryType::Auto;

        /// <summary>
        /// How the memory will be accessed.
        /// </summary>
        BufferMemoryUsage memoryUsage = BufferMemoryUsage::GpuOnly;

        /// <summary>
        /// How the buffer can be shared between queue families.
        /// </summary>
        SharingMode sharing = SharingMode::Exclusive;

        /// <summary>
        /// The number of levels of detail available for minified sampling of the image.
        /// Set to 1 for a standard texture without any mipmaps.
        /// </summary>
        uint32_t mipLevels = 1u;

        /// <summary>
        /// If a texture array, the number of array layers.
        /// Set to 1 for a standard, single-layered texture.
        /// </summary>
        uint32_t arrayLayers = 1u;

        uint32_t faceCount = 1u;

        /// <summary>
        /// The number of samples per pixel. Primarily used for anti-aliasing.
        /// Set to a count of 1 for no multisampling.
        /// </summary>
        MultisampleCount sampleCount = MultisampleCount::Count1;

        /// <summary>
        /// Is the texture to be intrepeted as a cube map?
        /// </summary>
        bool isCubeMap = false;

        /// <summary>
        /// Does this texture reside in the global bindless texture table?
        /// </summary>
        bool residesInTextureTable = true;

        /// <summary>
        /// Optional name for the texture.
        /// If specified, it needs to be unique (or the original resource path).
        /// Used for hotreloads and debugging.
        /// </summary>
        std::string name;

        void calculateDimensionality() noexcept
        {
            dimensions = TextureDimensions::Texture3D;

            if (depth == 1u)
            {
                if (height == 1u)
                {
                    dimensions = TextureDimensions::Texture1D;
                }
                else
                {
                    dimensions = TextureDimensions::Texture2D;
                }
            }
        }
    };

    struct TextureResourceTag {};
    using TextureResourceHandle = Handle<TextureResourceTag>;

    struct MappedTexture
    {
        /// <summary>
        /// If the texture is mapped, this is the CPU address of the start of its memory block.
        /// </summary>
        void* mappedPtr = nullptr;
    };

    struct TextureUploadRegion
    {
        /// <summary>
        /// Offset into the source data buffer that we are reading from.
        /// </summary>
        uint64_t sourceOffset{ 0ull };

        /// <summary>
        /// The mipmap level that we are writing to. Level 0 is the top-most level with full resolution.
        /// </summary>
        uint32_t mipLevel{ 0u };

        /// <summary>
        /// The array layer index that we are writing to.
        /// </summary>
        uint32_t arrayLayer{ 0u };

        /// <summary>
        /// Width of the region in pixels.
        /// </summary>
        uint32_t width{ 1u };

        /// <summary>
        /// Height of the region in pixels.
        /// </summary>
        uint32_t height{ 1u };

        /// <summary>
        /// Depth of the region in pixels.
        /// </summary>
        uint32_t depth{ 1u };
    };

    /// <summary>
    /// Series of reserved indices in the global texture table for common fallback textures.
    /// </summary>
    enum class TextureTableReservedIndices : uint32_t
    {
        /// <summary>
        /// A 1x1 white (1.0, 1.0, 1.0, 1.0) texture.
        /// </summary>
        White = 0u,

        /// <summary>
        /// A 1x1 black (0.0, 0.0, 0.0, 1.0) texture.
        /// </summary>
        Black = 1u,

        /// <summary>
        /// A 1x1 pink (1.0, 0.0, 1.0, 1.0) texture commonly used to indicate a missing texture.
        /// </summary>
        Pink = 2u,

        /// <summary>
        /// A 1x1 passive tangent normal texture (0.5, 0.5, 0.5, 1.0)
        /// </summary>
        TangentNormal = 3u,

        ReservedIndicesCount
    };

    /// <summary>
    /// Builds the region set for a tightly packed source buffer holding every mip of every layer, ordered level-major then layer. One region per subresource.
    /// </summary>
    void buildTightlyPackedUploadRegions(TextureResourceDescriptor const& descriptor, std::vector<TextureUploadRegion>& outRegions) noexcept;
}

#endif