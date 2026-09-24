#include "litl-renderer/resources/texture.hpp"

namespace litl
{
    void buildTightlyPackedUploadRegions(TextureResourceDescriptor const& descriptor, std::vector<TextureUploadRegion>& outRegions) noexcept
    {
        outRegions.reserve(static_cast<size_t>(descriptor.mipLevels) * descriptor.arrayLayers);

        uint64_t offset = 0ull;

        for (uint32_t level = 0u; level < descriptor.mipLevels; ++level)
        {
            const uint32_t levelWidth  = mipExtent(descriptor.width, level);
            const uint32_t levelHeight = mipExtent(descriptor.height, level);
            const uint32_t levelDepth  = mipExtent(descriptor.depth, level);
            const uint64_t layerBytes  = imageLevelBytes(descriptor.format, levelWidth, levelHeight, levelDepth);

            for (uint32_t layer = 0u; layer < descriptor.arrayLayers; ++layer)
            {
                outRegions.push_back(TextureUploadRegion{
                    .sourceOffset = offset,
                    .mipLevel = level,
                    .arrayLayer = layer,
                    .width = levelWidth,
                    .height = levelHeight,
                    .depth = levelDepth
                });

                offset += layerBytes;
            }
        }
    }
}