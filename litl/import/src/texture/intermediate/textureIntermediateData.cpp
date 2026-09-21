#include <array>

#include "litl-core/formats/srgb.hpp"
#include "litl-core/math/textureUtils.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-import/texture/intermediate/textureIntermediateData.hpp"

namespace litl::import
{
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
        if (pixels.size() != (m_dataDescriptor.width * m_dataDescriptor.height * m_dataDescriptor.depth * 4u))      // * 4 as expected to be RGBA
        {
            // Data descriptor is out-of-sync with the data being provided.
            return false;
        }

        const uint32_t textureLevelCount = mipLevelCount(m_dataDescriptor.width, m_dataDescriptor.height, m_dataDescriptor.depth);

        m_levels.clear();
        m_levels.reserve(textureLevelCount);
        m_levels.push_back(TextureLevel{
            .byteOffset = 0ull,
            .byteSize = pixels.size() * 4u,             // * 4 as we will be expanding from std::byte/uint8_t to float
            .width = m_dataDescriptor.width,
            .height = m_dataDescriptor.height,
            .depth = m_dataDescriptor.depth
        });

        if (m_dataDescriptor.mipMaps)
        {
            for (uint32_t i = 1u; i < textureLevelCount; ++i)
            {
                const uint32_t levelWidth = mipExtent(m_dataDescriptor.width, i);
                const uint32_t levelHeight = mipExtent(m_dataDescriptor.height, i);
                const uint32_t levelDepth = mipExtent(m_dataDescriptor.depth, i);

                m_levels.push_back(TextureLevel{
                    .byteOffset = (m_levels[i - 1].byteOffset + m_levels[i - 1].byteSize),
                    .byteSize = (levelWidth * levelHeight * levelDepth * 4u),
                    .width = levelWidth,
                    .height = levelHeight,
                    .depth = levelDepth
                });
            }
        }

        m_pixels.clear();
        m_pixels.resize(m_levels.back().byteOffset + m_levels.back().byteSize, std::byte{ 0 });

        std::span<float> pixelsReinterp{ reinterpret_cast<float*>(m_pixels.data()), pixels.size() * 4u };

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
        if (m_pixels.empty())
        {
            logError("Failed to validate TextureIntermediateData: empty pixels buffer.");
            return false;
        }

        if ((m_dataDescriptor.width == 0u) ||
            (m_dataDescriptor.height == 0u) ||
            (m_dataDescriptor.depth == 0u) ||
            (m_dataDescriptor.arrayLayers == 0u ||
            (m_dataDescriptor.faceCount == 0u)))
        {
            logError("Failed to validate TextureIntermediateData: invalid extents. Width = ", m_dataDescriptor.width, ", Height = ", m_dataDescriptor.height, ", Depth = ", m_dataDescriptor.depth, ", Layers = ", m_dataDescriptor.arrayLayers);
            return false;
        }

        if ((m_dataDescriptor.format != DataFormat::RGBA32_SFloat) &&
            (m_dataDescriptor.format != DataFormat::RGBA8_SRGB) &&
            (m_dataDescriptor.format != DataFormat::RGBA8_UNorm) &&
            (m_dataDescriptor.format != DataFormat::RGBA16_SFloat))
        {
            logError("Failed to validate TextureIntermediateData: invalid format ", static_cast<uint32_t>(m_dataDescriptor.format), ". Expected RGBA8_SRGB, RGBA8_UNorm, or RGBA16_SFloat.");
            return false;
        }

        if ((m_dataDescriptor.semantic == TextureSemantic::Unknown) ||
            (static_cast<uint32_t>(m_dataDescriptor.semantic) > static_cast<uint32_t>(TextureSemantic::Hdr)))
        {
            logError("Failed to validate TextureIntermediateData: invalid semantic of ", static_cast<uint32_t>(m_dataDescriptor.semantic));
            return false;
        }

        if ((m_pixels.size() % (sizeof(float) * 4)) != 0)
        {
            logError("Failed to validate TextureIntermediateData: invalid pixels buffer size. Expected multiple of sizeof(float) * 4. Forced float and forced RGBA required.");
            return false;
        }

        if (m_levels.empty())
        {
            logError("Failed to validate TextureIntermediateData: texture levels empty.");
            return false;
        }
        else
        {
            if (m_dataDescriptor.mipMaps)
            {
                if (m_levels.size() == 1)
                {
                    logError("Failed to validate TextureIntermediateData: mipmaps enabled in data descriptor, but levels beyond index 0 are missing.");
                    return false;
                }
            }
            else
            {
                if (m_levels.size() > 1)
                {
                    logError("Failed to validate TextureIntermediateData: mipmaps disabled in data descriptor, but levels beyond index 0 are present.");
                    return false;
                }
            }
        }

        return true;
    }

    uint32_t TextureIntermediateData::levelsCount() const noexcept
    {
        return static_cast<uint32_t>(m_levels.size());
    }

    bool TextureIntermediateData::generateMipMaps() noexcept
    {
        if (!m_dataDescriptor.mipMaps)
        {
            // Nothing to do.
            return true;
        }

        // ... todo ...
        logError("TextureIntermediateData::generateMipMaps for m_dataDescriptor.mipMaps == true is unimplemented.");

        return false;
    }
}