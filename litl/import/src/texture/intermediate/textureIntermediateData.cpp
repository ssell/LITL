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
}