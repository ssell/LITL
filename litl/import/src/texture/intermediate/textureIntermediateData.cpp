#include <array>
#include <memory>

#include "litl-core/formats/srgb.hpp"
#include "litl-core/math/textureUtils.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-import/texture/intermediate/textureIntermediateData.hpp"

namespace litl::import
{
    namespace
    {
        static constexpr size_t ComponentCount = 4;         // RGBA
    }

    TextureDataDescriptor& TextureIntermediateData::getDataDescriptorWriteRef() noexcept
    {
        return m_dataDescriptor;
    }

    TextureDataDescriptor const& TextureIntermediateData::getDataDescriptor() const noexcept
    {
        return m_dataDescriptor;
    }

    std::vector<TextureLevel>& TextureIntermediateData::getTextureLevelsWriteRef() noexcept
    {
        return m_levels;
    }

    std::span<TextureLevel const> TextureIntermediateData::getTextureLevels() const noexcept
    {
        return m_levels;
    }

    std::vector<std::byte>& TextureIntermediateData::getPixelBytesWriteRef() noexcept
    {
        return m_pixels;
    }

    std::span<std::byte const> TextureIntermediateData::getPixelBytes() const noexcept
    {
        return m_pixels;
    }

    bool TextureIntermediateData::store8BitPixelsAsFloat(std::span<std::byte const> pixels) noexcept
    {
        if (pixels.size() != imageLevelBytes(DataFormat::RGBA8_UNorm, m_dataDescriptor.width, m_dataDescriptor.height, m_dataDescriptor.depth))     // This function expects the incoming pixel data to be 8-bits per channel with 4 channels (RGBA)
        {
            // Data descriptor is out-of-sync with the data being provided.
            return false;
        }

        const uint32_t textureLevelCount = mipLevelCount(m_dataDescriptor.width, m_dataDescriptor.height, m_dataDescriptor.depth);

        m_levels.clear();
        m_levels.reserve(textureLevelCount);
        m_levels.push_back(TextureLevel{
            .byteOffset = 0ull,
            .byteSize = pixels.size() * sizeof(float),
            .width = m_dataDescriptor.width,
            .height = m_dataDescriptor.height,
            .depth = m_dataDescriptor.depth
        });

        if (m_dataDescriptor.mipmaps)
        {
            for (uint32_t i = 1u; i < textureLevelCount; ++i)
            {
                const uint32_t levelWidth = mipExtent(m_dataDescriptor.width, i);
                const uint32_t levelHeight = mipExtent(m_dataDescriptor.height, i);
                const uint32_t levelDepth = mipExtent(m_dataDescriptor.depth, i);

                m_levels.push_back(TextureLevel{
                    .byteOffset = (m_levels[i - 1].byteOffset + m_levels[i - 1].byteSize),
                    .byteSize = (levelWidth * levelHeight * levelDepth * sizeof(float) * ComponentCount),
                    .width = levelWidth,
                    .height = levelHeight,
                    .depth = levelDepth
                });
            }
        }

        m_pixels.clear();
        m_pixels.resize(m_levels.back().byteOffset + m_levels.back().byteSize, std::byte{ 0 });


        std::span<float> pixelsReinterp{ reinterpret_cast<float*>(m_pixels.data()), pixels.size() };

        const auto& byteToFloatTransferTable = getByteToFloatTable(m_dataDescriptor.transfer);
        const auto& byteToLinearFloatTable = getByteToLinearFloatTable();

        for (uint32_t i = 0u; i < static_cast<uint32_t>(pixels.size()); i += 4u)
        {
            pixelsReinterp[i + 0] = byteToFloatTransferTable[static_cast<uint8_t>(pixels[i + 0])];
            pixelsReinterp[i + 1] = byteToFloatTransferTable[static_cast<uint8_t>(pixels[i + 1])];
            pixelsReinterp[i + 2] = byteToFloatTransferTable[static_cast<uint8_t>(pixels[i + 2])];
            pixelsReinterp[i + 3] = byteToLinearFloatTable[static_cast<uint8_t>(pixels[i + 3])];            // alpha is linear
        }

        // Levels beyond 0 (mipmaps) are left unpopulated. It is up to the user to call generateMipMaps.

        return true;
    }

    bool TextureIntermediateData::validate() const noexcept
    {
        // -----------------------------------------------------------------------------
        // Descriptor
        // -----------------------------------------------------------------------------

        if ((m_dataDescriptor.width == 0u) ||
            (m_dataDescriptor.height == 0u) ||
            (m_dataDescriptor.depth == 0u) ||
            (m_dataDescriptor.arrayLayers == 0u) ||
            (m_dataDescriptor.faceCount == 0u))
        {
            logError("Failed to validate TextureIntermediateData: invalid extents. Width = ", m_dataDescriptor.width, ", Height = ", m_dataDescriptor.height, ", Depth = ", m_dataDescriptor.depth, ", Layers = ", m_dataDescriptor.arrayLayers, ", Faces = ", m_dataDescriptor.faceCount, ". All must be non-zero.");
            return false;
        }

        if ((m_dataDescriptor.format != DataFormat::RGBA32_SFloat) &&
            (m_dataDescriptor.format != DataFormat::RGBA16_SFloat) &&
            (m_dataDescriptor.format != DataFormat::RGBA8_SRGB) &&
            (m_dataDescriptor.format != DataFormat::RGBA8_UNorm))
        {
            logError("Failed to validate TextureIntermediateData: unsupported format ", static_cast<uint32_t>(m_dataDescriptor.format), ". Expected RGBA32_SFloat, RGBA16_SFloat, RGBA8_SRGB, or RGBA8_UNorm.");
            return false;
        }

        if ((m_dataDescriptor.semantic == TextureSemantic::Unknown) ||
            (static_cast<uint32_t>(m_dataDescriptor.semantic) > static_cast<uint32_t>(TextureSemantic::Hdr)))
        {
            logError("Failed to validate TextureIntermediateData: invalid semantic of ", static_cast<uint32_t>(m_dataDescriptor.semantic), ".");
            return false;
        }

        if ((m_dataDescriptor.transfer != TransferFunction::Linear) &&
            (m_dataDescriptor.transfer != TransferFunction::SRGB))
        {
            logError("Failed to validate TextureIntermediateData: invalid transfer function of ", static_cast<uint32_t>(m_dataDescriptor.transfer), ".");
            return false;
        }

        // A cube map is exactly six faces; everything else is exactly one. Keeps faceCount from silently
        // disagreeing with isCubeMap, which would make every derived subresource count wrong.
        const uint32_t requiredFaceCount = (m_dataDescriptor.isCubeMap ? 6u : 1u);

        if (m_dataDescriptor.faceCount != requiredFaceCount)
        {
            logError("Failed to validate TextureIntermediateData: faceCount of ", m_dataDescriptor.faceCount, " does not match isCubeMap of ", m_dataDescriptor.isCubeMap, ". Expected ", requiredFaceCount, ".");
            return false;
        }

        // -----------------------------------------------------------------------------
        // Level Table
        // -----------------------------------------------------------------------------

        if (m_pixels.empty())
        {
            logError("Failed to validate TextureIntermediateData: empty pixels buffer.");
            return false;
        }

        if (m_levels.empty())
        {
            logError("Failed to validate TextureIntermediateData: texture levels empty. Level 0 is always required.");
            return false;
        }

        const uint32_t expectedLevelCount = (m_dataDescriptor.mipmaps ? mipLevelCount(m_dataDescriptor.width, m_dataDescriptor.height, m_dataDescriptor.depth) : 1u);

        if (m_levels.size() != static_cast<size_t>(expectedLevelCount))
        {
            logError("Failed to validate TextureIntermediateData: level count of ", m_levels.size(), " does not match the expected count of ", expectedLevelCount, " for mipmaps = ", m_dataDescriptor.mipmaps, ".");
            return false;
        }

        // Every level holds one image per array layer per cube face.
        const uint64_t subresourceCount = static_cast<uint64_t>(m_dataDescriptor.arrayLayers) * static_cast<uint64_t>(m_dataDescriptor.faceCount);

        // Each level begins on a whole texel block, which is what vkCmdCopyBufferToImage requires of bufferOffset
        // (a multiple of 4 and of the texel block size). Tight packing satisfies this for every supported format
        // as each level is a whole number of blocks, but it is checked explicitly so a hand-built level table cannot slip through.
        const uint64_t levelAlignment = dataFormatBufferCopyAlignment(m_dataDescriptor.format);

        uint64_t runningOffset = 0ull;

        for (uint32_t i = 0u; i < static_cast<uint32_t>(m_levels.size()); ++i)
        {
            TextureLevel const& level = m_levels[i];

            const uint32_t expectedWidth = mipExtent(m_dataDescriptor.width, i);
            const uint32_t expectedHeight = mipExtent(m_dataDescriptor.height, i);
            const uint32_t expectedDepth = mipExtent(m_dataDescriptor.depth, i);

            if ((level.width != expectedWidth) || (level.height != expectedHeight) || (level.depth != expectedDepth))
            {
                logError("Failed to validate TextureIntermediateData: level ", i, " has extents ", level.width, "x", level.height, "x", level.depth, " but expected ", expectedWidth, "x", expectedHeight, "x", expectedDepth, ".");
                return false;
            }

            const uint64_t expectedByteSize = imageLevelBytes(m_dataDescriptor.format, expectedWidth, expectedHeight, expectedDepth) * subresourceCount;

            if (level.byteSize != expectedByteSize)
            {
                logError("Failed to validate TextureIntermediateData: level ", i, " declares ", level.byteSize, " bytes but its extents and format require ", expectedByteSize, ".");
                return false;
            }

            // Levels tile the pixel buffer exactly: no gaps, no overlaps, in ascending order.
            if (level.byteOffset != runningOffset)
            {
                logError("Failed to validate TextureIntermediateData: level ", i, " begins at byte ", level.byteOffset, " but the preceding levels end at ", runningOffset, ".");
                return false;
            }

            if ((level.byteOffset % levelAlignment) != 0ull)
            {
                logError("Failed to validate TextureIntermediateData: level ", i, " begins at byte ", level.byteOffset, " which is not a multiple of the ", levelAlignment, " byte texel block size.");
                return false;
            }

            runningOffset += level.byteSize;
        }

        if (runningOffset != static_cast<uint64_t>(m_pixels.size()))
        {
            logError("Failed to validate TextureIntermediateData: level table covers ", runningOffset, " bytes but the pixels buffer holds ", m_pixels.size(), ".");
            return false;
        }

        return true;
    }

    uint32_t TextureIntermediateData::levelsCount() const noexcept
    {
        return static_cast<uint32_t>(m_levels.size());
    }

    bool TextureIntermediateData::generateMipMaps() noexcept
    {
        if (!m_dataDescriptor.mipmaps)
        {
            // Nothing to do.
            return true;
        }

        // ... todo ...
        logError("TextureIntermediateData::generateMipMaps for m_dataDescriptor.mipMaps == true is unimplemented.");

        return false;
    }
}