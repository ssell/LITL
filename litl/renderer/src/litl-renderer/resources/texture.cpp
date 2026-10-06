#include "litl-renderer/resources/texture.hpp"
#include "litl-core/string.hpp"

namespace litl
{
    namespace
    {
        constexpr std::array<std::string_view, static_cast<uint32_t>(TextureTableReservedIndices::ReservedIndicesCount)> g_TextureTableReservedIndexNames{
            "pink",
            "white",
            "black",
            "normal"
        };
    }

    std::optional<TextureTableReservedIndices> getReservedTextureTableIndex(std::string_view name) noexcept
    {
        const std::string_view* __restrict preservedNames = g_TextureTableReservedIndexNames.data();

        for (size_t i = 0ull; i < g_TextureTableReservedIndexNames.size(); ++i)
        {
            if (stringsEqualFirstLowercase(preservedNames[i], name))
            {
                return static_cast<TextureTableReservedIndices>(i);
            }
        }

        return std::nullopt;
    }

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