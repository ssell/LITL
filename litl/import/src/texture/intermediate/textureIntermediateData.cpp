#include <array>

#include "litl-core/formats/srgb.hpp"
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

    bool TextureIntermediateData::store8BitPixelsAsFloat(std::span<uint8_t const> pixels) noexcept
    {
        if ((pixels.size() % 4) != 0u)
        {
            // Must be RGBA
            return false;
        }

        m_pixels.clear();
        m_pixels.resize(pixels.size() * 4u);

        std::span<float> pixelsReinterp{ reinterpret_cast<float*>(m_pixels.data()), pixels.size() * 4u };

        if (m_dataDescriptor.transfer == TransferFunction::Linear)
        {
            for (uint32_t i = 0u; i < static_cast<uint32_t>(pixels.size()); ++i)
            {
                pixelsReinterp[i] = uint8_to_linear_float[pixels[i]];
            }
        }
        else
        {
            for (uint32_t i = 0u; i < static_cast<uint32_t>(pixels.size()); i += 4u)
            {
                pixelsReinterp[i + 0] = uint8_to_srgb_float[pixels[i + 0]];
                pixelsReinterp[i + 1] = uint8_to_srgb_float[pixels[i + 1]];
                pixelsReinterp[i + 2] = uint8_to_srgb_float[pixels[i + 2]];
                pixelsReinterp[i + 3] = uint8_to_linear_float[pixels[i + 3]];           // alpha is linear
            }
        }

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
            (m_dataDescriptor.arrayLayers == 0u))
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

        return true;
    }
}