#include <array>

#include "litl-import/texture/intermediate/litlbtex.hpp"
#include "litl-core/containers/common.hpp"

namespace litl::import
{
    namespace
    {
        struct BinaryTextureLevel
        {
            uint64_t byteOffset{ 0ull };                // Relative to the start of the PIXL block.
            uint64_t byteSize{ 0ull };                  // Bytes for this level across all layers and faces.
            uint64_t uncompressedByteSize{ 0ull };      // Future facing for zstd
            uint32_t width{ 1u };
            uint32_t height{ 1u };
            uint32_t depth{ 1u };
            std::array<uint32_t, 3> padding;
        };

        static_assert(sizeof(BinaryTextureLevel) == 48);
        static_assert(sizeof(BinaryTextureLevel) % 16 == 0);
        static_assert(std::is_trivially_copyable_v<BinaryTextureLevel>);

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
            uint32_t padding{ 0u };
        };

        static_assert(sizeof(BinaryTextureDataDescriptor) == 32);
        static_assert(sizeof(BinaryTextureDataDescriptor) % 16 == 0);
        static_assert(std::is_trivially_copyable_v<BinaryTextureDataDescriptor>);

        [[nodiscard]] BinaryTextureLevel serializeTextureLevel(TextureLevel level) noexcept
        {
            return BinaryTextureLevel{
                .byteOffset = level.byteOffset,
                .byteSize = level.byteSize,
                .uncompressedByteSize = 0ull,
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

        [[nodiscard]] BinaryTextureDataDescriptor serializeTextureDataDescriptor(TextureDataDescriptor desc) noexcept
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
                .alphaPremultiplied = desc.alphaPremultiplied
            };
        }
    }

    bool LitlTextureBinary::serialize(TextureIntermediateData const& texture, std::vector<std::byte>& data, ErrorCode& error) noexcept
    {
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
        // ... todo ...
        error = ErrorCode::FunctionNotImplemented;
        return false;
    }
}