#include <algorithm>
#include <cstring>

#include "litl-core/hash.hpp"
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
                error = ErrorCode::BlockSizeMismatch;
                return false;
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

        runningBlockOffset += data.descriptor->blockBytes;
    }

    // -------------------------------------------------------------------------------------
    // Utility
    // -------------------------------------------------------------------------------------

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
}