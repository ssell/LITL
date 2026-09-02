#include <algorithm>
#include <cstring>

#include "litl-core/hash.hpp"
#include "litl-core/containers/common.hpp"
#include "litl-core/math/common.hpp"
#include "litl-core/formats/binaryBlockFile.hpp"
#include "litl-core/formats/binaryBlobReader.hpp"

namespace litl
{
    uint64_t BinaryBlockFile::calculateContentHash(std::span<std::byte const> data, Header const& header) noexcept
    {
        return hashSubarray(data, sizeof(Header), (header.totalBytes - sizeof(Header)));
    }

    // -------------------------------------------------------------------------------------
    // Parsing
    // -------------------------------------------------------------------------------------

    bool BinaryBlockFile::Header::validate(ErrorCode& error, BinaryBlockFileFormatIdentity const& identity) const noexcept
    {
        if (magic != identity.magic)
        {
            error = ErrorCode::InvalidFileType;
            return false;
        }

        if (versionMajor != identity.versionMajor)
        {
            error = ErrorCode::MajorVersionMismatch;
            return false;
        }

        if (versionMinor > identity.versionMinor)
        {
            error = ErrorCode::MinorVersionMismatch;
            return false;
        }

        if (totalBytes < sizeof(Header))
        {
            error = ErrorCode::InvalidFileSize;
            return false;
        }

        if (blockCount == 0u)
        {
            error = ErrorCode::WhereTheBlocksAt;
            return false;
        }

        if (blockCount > MaxBlocks)
        {
            error = ErrorCode::TooManyBlocks;
            return false;
        }

        if ((descriptorsOffset < sizeof(Header)) || (descriptorsOffset > totalBytes))
        {
            error = ErrorCode::InvalidFirstDescriptorOffset;
            return false;
        }

        if (blocksOffset != (descriptorsOffset + (blockCount * sizeof(BlockDescriptor))))
        {
            error = ErrorCode::InvalidFirstBlockOffset;
            return false;
        }

        if ((blocksOffset % 16u) != 0u)
        {
            error = ErrorCode::InvalidFirstBlockOffset;
            return false;
        }

        return true;
    }

    bool BinaryBlockFile::parseImpl(std::span<std::byte const> data, BinaryBlockFileFormatIdentity const& identity, BinaryBlockFile& file, ErrorCode& error) noexcept
    {
        BinaryBlockFile parsed{};
        error = ErrorCode::None;

        if (data.size() < sizeof(Header))
        {
            error = ErrorCode::InvalidFileSize;
            return false;
        }

        std::memcpy(&parsed.header, data.data(), sizeof(Header));

        if (!parsed.header.validate(error, identity))
        {
            return false;
        }

        if (data.size() != parsed.header.totalBytes)
        {
            error = ErrorCode::InvalidFileSize;
            return false;
        }

        auto const contentHash = calculateContentHash(data, parsed.header);

        if (contentHash != parsed.header.contentHash)
        {
            error = ErrorCode::ContentHashMismatch;
            return false;
        }

        auto const descriptorBytes = uint64_t{ parsed.header.blockCount } * sizeof(BlockDescriptor);

        if (descriptorBytes > (parsed.header.totalBytes - parsed.header.descriptorsOffset))
        {
            error = ErrorCode::InvalidFileSize;
            return false;
        }

        BinaryBlobReader reader({ data.data() + parsed.header.descriptorsOffset, descriptorBytes });
        BlockDescriptor currDescriptor{};
        uint64_t prevBlockEnd = 0u;

        for (uint32_t i = 0u; i < parsed.header.blockCount; ++i)
        {
            if (!reader.read(currDescriptor))
            {
                error = ErrorCode::MissingBlockDescriptor;
                return false;
            }

            if ((currDescriptor.blockOffset < parsed.header.blocksOffset) || (currDescriptor.blockOffset > parsed.header.totalBytes))
            {
                error = ErrorCode::DescriptorBlockOutOfBounds;
                return false;
            }

            if (currDescriptor.blockBytes > (parsed.header.totalBytes - currDescriptor.blockOffset))
            {
                error = ErrorCode::BlockSizeOutOfBounds;
                return false;
            }

            if (currDescriptor.elementBytes == 0u)
            {
                error = ErrorCode::ElementSizeOfZero;
                return false;
            }

            if ((uint64_t{ currDescriptor.elementBytes } * currDescriptor.elementCount) != currDescriptor.blockBytes)
            {
                // The one exception is an empty block which has 0 elements and a size of 16 to enforce space between blocks to avoid overlap.
                if ((currDescriptor.blockBytes != 16) || (currDescriptor.elementCount != 0))
                {
                    error = ErrorCode::BlockSizeMismatch;
                    return false;
                }
            }

            if ((currDescriptor.blockOffset % 16) != 0)
            {
                error = ErrorCode::InvalidBlockOffset;
                return false;
            }

            if (currDescriptor.blockOffset < prevBlockEnd)
            {
                error = ErrorCode::BlockOverlap;
                return false;
            }

            parsed.descriptors[i] = currDescriptor;
            prevBlockEnd = currDescriptor.blockOffset + currDescriptor.blockBytes;
        }

        parsed.data = data;
        file = parsed;
        return true;
    }

    // -------------------------------------------------------------------------------------
    // Serialization
    // -------------------------------------------------------------------------------------

    void BinaryBlockFile::serializeBlock(BlockDataDescriptor& data, uint64_t& runningBlockOffset) noexcept
    {
        // Ensure our offsets remain a multiple of 16
        runningBlockOffset = alignMemoryOffsetUp(runningBlockOffset, 16);

        data.descriptor->blockId = data.id;
        data.descriptor->elementBytes = data.elementSize;
        data.descriptor->elementCount = data.data.size() / data.elementSize;
        data.descriptor->blockOffset = runningBlockOffset;
        data.descriptor->blockBytes = data.data.size();
        data.descriptor->flags = 0u;

        if (data.data.size() == 0)
        {
            // An empty block still needs space between it and the next block to avoid overlaps.
            data.descriptor->blockBytes = 16u;
        }

        runningBlockOffset += data.descriptor->blockBytes;
    }

    BinaryBlockFile::StringRef BinaryBlockFile::serializeString(std::string_view string, StringMap& stringMap) noexcept
    {
        const auto stringId = StringId(string);
        const auto find = stringMap.map.find(stringId);

        if (find != stringMap.map.end())
        {
            return stringMap.stringRefs[find->second];
        }

        const auto offset = stringMap.runningOffset;

        stringMap.map[stringId] = stringMap.stringRefs.size();
        stringMap.strings.push_back(std::string(string));
        stringMap.stringRefs.push_back(StringRef{ .offset = offset, .length = static_cast<uint32_t>(string.size()) });
        stringMap.runningOffset = stringMap.runningOffset + string.size();

        return stringMap.stringRefs.back();
    }

    std::string_view BinaryBlockFile::deserializeString(std::span<char const> strings, BinaryBlockFile::StringRef ref, ErrorCode& error) noexcept
    {
        if (ref.length == 0u)
        {
            error = ErrorCode::StringRefEmpty;
            return {};
        }

        if ((ref.offset + ref.length) > strings.size_bytes())
        {
            error = ErrorCode::StringRefOutOfBounds;
            return {};
        }

        return std::string_view{ strings.data() + ref.offset, ref.length };
    }

    // -------------------------------------------------------------------------------------
    // Utility
    // -------------------------------------------------------------------------------------

    bool BinaryBlockFile::addDataBlockDescriptor(std::vector<BinaryBlockFile::BlockDataDescriptor>& blockDataTable, uint32_t descriptorIndex, BinaryBlockIdType const& id, size_t elementSize, std::span<std::byte const> data, BinaryBlockFile::ErrorCode& error) noexcept
    {
        if ((descriptorIndex == BinaryBlockFile::MaxBlocks) || (blockDataTable.size() == BinaryBlockFile::MaxBlocks))
        {
            error = BinaryBlockFile::ErrorCode::TooManyBlocks;
            return false;
        }

        blockDataTable.push_back(BinaryBlockFile::BlockDataDescriptor{
            .descriptor = &descriptors[descriptorIndex],
            .id = id,
            .elementSize = elementSize,
            .data = data
        });

        return true;
    }

    void BinaryBlockFile::addDefaultBlockDescriptors(std::vector<BlockDataDescriptor>& blockDataTable) noexcept
    {
        blockDataTable.push_back(BlockDataDescriptor{
            .descriptor = &descriptors[DefaultBlocks::StringsBlockIndex],
            .id = DefaultBlocks::Strings,
            .elementSize = sizeof(char),
            .data = {}
        });
    }

    void BinaryBlockFile::serializeDefaultBlocks(std::vector<BlockDataDescriptor>& blockDataTable, StringMap& stringMap) noexcept
    {
        if (!stringMap.map.empty())
        {
            stringMap.stringBlob.resize(stringMap.runningOffset, std::byte{ 0 });
            size_t offset = 0;

            for (auto& string : stringMap.strings)
            {
                std::memcpy(stringMap.stringBlob.data() + offset, string.data(), string.size());
                offset += string.size();
            }

            blockDataTable[DefaultBlocks::StringsBlockIndex].data = { stringMap.stringBlob.data(), stringMap.stringBlob.size() };
        }
    }

    std::optional<BinaryBlockFile::Block> BinaryBlockFile::find(BinaryBlockIdType id) const noexcept
    {
        for (uint32_t i = 0u; i < header.blockCount; ++i)
        {
            auto& descriptor = descriptors[i];

            if (descriptor.blockId == id)
            {
                return Block{
                    .blockId = descriptor.blockId,
                    .elementBytes = descriptor.elementBytes,
                    .elementCount = descriptor.elementCount,
                    .bytes = std::span<std::byte const>{ data.data() + descriptor.blockOffset, descriptor.blockBytes }
                };
            }
        }

        return std::nullopt;
    }

    void BinaryBlockFile::serializeDataBuffer(BinaryBlockFile& blockFile, std::vector<BlockDataDescriptor>& blockDataTable, std::vector<std::byte>& data) noexcept
    {
        data.resize(blockFile.header.totalBytes);
        std::fill(data.begin(), data.end(), std::byte(0));

        // Copy the descriptors
        for (size_t i = 0ull; i < blockDataTable.size(); ++i)
        {
            std::memcpy(data.data() + blockFile.header.descriptorsOffset + (sizeof(BlockDescriptor) * i), blockDataTable[i].descriptor, sizeof(BlockDescriptor));
        }

        // Copy the data
        for (auto& blockData : blockDataTable)
        {
            if (blockData.data.size() > 0)
            {
                std::memcpy(data.data() + blockData.descriptor->blockOffset, blockData.data.data(), blockData.data.size());
            }
        }

        // Calculate hash and then copy the header
        blockFile.header.contentHash = calculateContentHash(std::span<std::byte const>(data), blockFile.header);
        std::memcpy(data.data(), &blockFile.header, sizeof(Header));
    }
}