#include <array>
#include <cstring>

#include "litl-import/texture/intermediate/litlbtex.hpp"
#include "litl-core/containers/common.hpp"
#include "litl-core/math/textureUtils.hpp"

namespace litl::import
{
    namespace
    {
        struct BinaryTextureDataDescriptor
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
            bool mipmaps{ false };
            std::array<std::byte, 3> padding{};
        };

        static_assert(sizeof(BinaryTextureDataDescriptor) == 32);
        static_assert(sizeof(BinaryTextureDataDescriptor) % 16 == 0);
        static_assert(std::is_trivially_copyable_v<BinaryTextureDataDescriptor>);

        struct BinaryTextureLevel
        {
            uint64_t byteOffset{ 0ull };                // Relative to the start of the PIXL block.
            uint64_t byteSize{ 0ull };                  // Bytes for this level across all layers and faces.
            uint64_t uncompressedByteSize{ 0ull };      // Future facing for zstd
            uint32_t width{ 1u };
            uint32_t height{ 1u };
            uint32_t depth{ 1u };
            std::array<uint32_t, 3> padding{};
        };

        static_assert(sizeof(BinaryTextureLevel) == 48);
        static_assert(sizeof(BinaryTextureLevel) % 16 == 0);
        static_assert(std::is_trivially_copyable_v<BinaryTextureLevel>);

        struct LitlTextureDeserializationData
        {
            std::span<BinaryTextureDataDescriptor const> textureDescriptors;
            std::span<BinaryTextureLevel const> textureLevels;
            std::span<std::byte const> pixels;
        };

        [[nodiscard]] BinaryTextureLevel serializeTextureLevel(TextureLevel level) noexcept
        {
            return BinaryTextureLevel{
                .byteOffset = level.byteOffset,
                .byteSize = level.byteSize,
                .uncompressedByteSize = level.byteSize,         // Update in the future when compression is supported
                .width = level.width,
                .height = level.height,
                .depth = level.depth,
            };
        }

        [[nodiscard]] std::vector<BinaryTextureLevel> serializeTextureLevels(std::span<TextureLevel const> levels)
        {
            std::vector<BinaryTextureLevel> binaryLevels;
            binaryLevels.reserve(levels.size());

            for (auto& level : levels)
            {
                binaryLevels.push_back(serializeTextureLevel(level));
            }

            return binaryLevels;
        }

        [[nodiscard]] TextureLevel deserializeTextureLevel(BinaryTextureLevel const& level) noexcept
        {
            return TextureLevel{
                .byteOffset = level.byteOffset,
                .byteSize = level.byteSize,
                .width = level.width,
                .height = level.height,
                .depth = level.depth
            };
        }

        [[nodiscard]] BinaryTextureDataDescriptor serializeTextureDataDescriptor(TextureDataDescriptor const& desc) noexcept
        {
            return BinaryTextureDataDescriptor{
                .format = desc.format,
                .width = desc.width,
                .height = desc.height,
                .depth = desc.depth,
                .arrayLayers = desc.arrayLayers,
                .faceCount = desc.faceCount,
                .transfer = desc.transfer,
                .semantic = desc.semantic,
                .isCubeMap = desc.isCubeMap,
                .alphaPremultiplied = desc.alphaPremultiplied,
                .mipmaps = desc.mipmaps
            };
        }

        [[nodiscard]] TextureDataDescriptor deserializeTextureDataDescriptor(BinaryTextureDataDescriptor const& desc) noexcept
        {
            return TextureDataDescriptor{
                .format = desc.format,
                .width = desc.width,
                .height = desc.height,
                .depth = desc.depth,
                .arrayLayers = desc.arrayLayers,
                .faceCount = desc.faceCount,
                .transfer = desc.transfer,
                .semantic = desc.semantic,
                .isCubeMap = desc.isCubeMap,
                .alphaPremultiplied = desc.alphaPremultiplied,
                .mipmaps = desc.mipmaps
            };
        }

        [[nodiscard]] bool deserializeBinaryTextureData(TextureIntermediateData& texture, LitlTextureDeserializationData const& textureData, BinaryBlockFile::ErrorCode& error) noexcept
        {
            if (textureData.textureDescriptors.size() == 0u)
            {
                error = BinaryBlockFile::ErrorCode::TextureMissingDescriptorInfo;
                return false;
            }

            if (textureData.textureDescriptors.size() > 1)
            {
                error = BinaryBlockFile::ErrorCode::TextureTooManyDescriptors;
                return false;
            }

            if (textureData.textureLevels.size() == 0u)
            {
                error = BinaryBlockFile::ErrorCode::TextureMissingTextureLevels;
                return false;
            }

            if (textureData.pixels.size() == 0u)
            {
                error = BinaryBlockFile::ErrorCode::TextureMissingPixels;
                return false;
            }

            // -----------------------------------------------------------------------------
            // Deserialize Descriptor
            // -----------------------------------------------------------------------------

            auto& textureDescriptor = texture.getDataDescriptorWriteRef();
            textureDescriptor = deserializeTextureDataDescriptor(textureData.textureDescriptors[0]);

            const uint32_t expectedLevelsCount = (textureDescriptor.mipmaps ?
                mipLevelCount(textureDescriptor.width, textureDescriptor.height, textureDescriptor.depth) : 
                1u);

            if (textureData.textureLevels.size() != expectedLevelsCount)
            {
                error = BinaryBlockFile::ErrorCode::TextureLevelInvalidCount;
                return false;
            }

            const uint64_t expectedChainBytes = (textureDescriptor.mipmaps ?
                imageChainBytes(textureDescriptor.format, textureDescriptor.width, textureDescriptor.height, textureDescriptor.depth) : 
                imageLevelBytes(textureDescriptor.format, textureDescriptor.width, textureDescriptor.height, textureDescriptor.depth)) * textureDescriptor.arrayLayers * textureDescriptor.faceCount;
            
            if (textureData.pixels.size_bytes() != expectedChainBytes)
            {
                error = BinaryBlockFile::ErrorCode::TexturePixelsInvalidByteCount;
                return false;
            }

            // -----------------------------------------------------------------------------
            // Deserialize Levels
            // -----------------------------------------------------------------------------

            const uint64_t blockBytes = static_cast<uint64_t>(dataFormatSize(textureDescriptor.format));
            const uint64_t levelAlignment = (blockBytes < 4ull) ? 4ull : blockBytes;

            for (uint32_t i = 0u; i < static_cast<uint32_t>(textureData.textureLevels.size()); ++i)
            {
                const auto& currLevel = textureData.textureLevels[i];

                if ((currLevel.byteOffset % levelAlignment) != 0)
                {
                    error = BinaryBlockFile::ErrorCode::TextureLevelInvalidOffset;
                    return false;
                }

                if (i != 0u)
                {
                    const auto& prevLevel = textureData.textureLevels[i - 1];

                    if (currLevel.byteOffset != (prevLevel.byteOffset + prevLevel.byteSize))
                    {
                        error = BinaryBlockFile::ErrorCode::TextureLevelGapOrOverlap;
                        return false;
                    }
                }

                const uint32_t expW = mipExtent(textureDescriptor.width, i);
                const uint32_t expH = mipExtent(textureDescriptor.height, i);
                const uint32_t expD = mipExtent(textureDescriptor.depth, i);

                if ((currLevel.width != expW) || (currLevel.height != expH) || (currLevel.depth != expD))
                {
                    error = BinaryBlockFile::ErrorCode::TextureLevelInvalidExtents;
                    return false;
                }

                if (currLevel.byteSize != (imageLevelBytes(textureDescriptor.format, expW, expH, expD) * textureDescriptor.arrayLayers * textureDescriptor.faceCount))
                {
                    error = BinaryBlockFile::ErrorCode::TextureLevelInvalidBytes;
                    return false;
                }

                if ((currLevel.byteOffset + currLevel.byteSize) > textureData.pixels.size_bytes())
                {
                    error = BinaryBlockFile::ErrorCode::TextureLevelOutOfBounds;
                    return false;
                }
            }

            auto& textureLevels = texture.getTextureLevelsWriteRef();
            textureLevels.clear();
            textureLevels.reserve(textureData.textureLevels.size());

            for (uint32_t i = 0u; i < static_cast<uint32_t>(textureData.textureLevels.size()); ++i)
            {
                textureLevels.push_back(deserializeTextureLevel(textureData.textureLevels[i]));
            }

            // -----------------------------------------------------------------------------
            // Deserialize Pixels
            // -----------------------------------------------------------------------------

            auto& texturePixels = texture.getPixelBytesWriteRef();
            texturePixels.clear();
            texturePixels.assign(textureData.pixels.begin(), textureData.pixels.end());

            return true;
        }
    }

    bool LitlTextureBinary::serialize(TextureIntermediateData const& texture, std::vector<std::byte>& data, ErrorCode& error) noexcept
    {
        if (!texture.validate())
        {
            error = BinaryBlockFile::ErrorCode::TextureValidationFailed;
            return false;
        }

        LitlTextureBinary litlTexture{};
        BinaryBlockFile::StringMap stringMap{};

        std::vector<BlockDataDescriptor> blockDataTable; blockDataTable.reserve(MaxBlocks);
        litlTexture.addDefaultBlockDescriptors(blockDataTable);

        const auto binaryDescriptor = serializeTextureDataDescriptor(texture.getDataDescriptor());
        const auto binaryTextureLevels = serializeTextureLevels(texture.getTextureLevels());

        if (!litlTexture.addDataBlockDescriptor(blockDataTable, BinaryBlockFile::DefaultBlocks::DefaultBlocksCount + 0, BlockIds::Info, sizeof(BinaryTextureDataDescriptor), as_byte_span(binaryDescriptor), error) ||
            !litlTexture.addDataBlockDescriptor(blockDataTable, BinaryBlockFile::DefaultBlocks::DefaultBlocksCount + 1, BlockIds::Levels, sizeof(BinaryTextureLevel), as_byte_span(binaryTextureLevels), error) ||
            !litlTexture.addDataBlockDescriptor(blockDataTable, BinaryBlockFile::DefaultBlocks::DefaultBlocksCount + 2, BlockIds::Pixels, sizeof(std::byte), texture.getPixelBytes(), error))
        {
            return false;
        }

        for (auto& blockData : blockDataTable)
        {
            if (blockData.elementSize == 0ull)
            {
                error = ErrorCode::ElementSizeOfZero;
                return false;
            }

            if (blockData.data.size() % blockData.elementSize != 0)
            {
                error = ErrorCode::ElementBlockIsNotWhole;
                return false;
            }
        }

        // ---------------------------------------------------------------------------------
        // Populate Header (most of it)

        litlTexture.header.magic = Identity.magic;
        litlTexture.header.versionMajor = Identity.versionMajor;
        litlTexture.header.versionMinor = Identity.versionMinor;
        litlTexture.header.contentHash = 0ull;         // calculated further on
        litlTexture.header.totalBytes = 0u;            // calculated further on
        litlTexture.header.blockCount = static_cast<uint32_t>(blockDataTable.size());
        litlTexture.header.descriptorsOffset = sizeof(Header);
        litlTexture.header.blocksOffset = litlTexture.header.descriptorsOffset + (sizeof(BlockDescriptor) * litlTexture.header.blockCount);
        litlTexture.header.flags = 0u;

        // ---------------------------------------------------------------------------------
        // Populate BlockDescriptors

        litlTexture.serializeDefaultBlocks(blockDataTable, stringMap);

        uint64_t runningOffset = litlTexture.header.blocksOffset;

        for (uint32_t i = 0; i < litlTexture.header.blockCount; ++i)
        {
            auto& blockData = blockDataTable[i];
            serializeBlock(blockData, runningOffset);
        }

        litlTexture.header.totalBytes = runningOffset;

        // ---------------------------------------------------------------------------------
        // Copy content to the provided data buffer

        serializeDataBuffer(litlTexture, blockDataTable, data);

        return true;
    }

    bool LitlTextureBinary::deserialize(TextureIntermediateData& texture, ErrorCode& error) const noexcept
    {
        auto infoBlock = find(BlockIds::Info);
        auto levelsBlock = find(BlockIds::Levels);
        auto pixelsBlock = find(BlockIds::Pixels);

        if (!infoBlock.has_value()) { error = ErrorCode::MissingTextureInfoBlock; return false; }
        if (!levelsBlock.has_value()) { error = ErrorCode::MissingTextureLevelsBlock; return false; }
        if (!pixelsBlock.has_value()) { error = ErrorCode::MissingTexturePixelsBlock; return false; }

        LitlTextureDeserializationData binaryTextureData{};

        binaryTextureData.textureDescriptors = infoBlock->as<BinaryTextureDataDescriptor const>(error).value_or({});
        binaryTextureData.textureLevels = levelsBlock->as<BinaryTextureLevel const>(error).value_or({});
        binaryTextureData.pixels = pixelsBlock->as<std::byte const>(error).value_or({});

        if (error != ErrorCode::None)
        {
            return false;
        }

        if (!deserializeBinaryTextureData(texture, binaryTextureData, error))
        {
            return false;
        }

        return true;
    }
}