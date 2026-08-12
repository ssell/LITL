#include <cstring>

#include "litl-core/hash.hpp"
#include "litl-core/formats/litlmesh.hpp"
#include "litl-core/formats/binaryBlobReader.hpp"

namespace litl
{
    bool LitlMesh::Header::validate(ErrorCode& error) const noexcept
    {
        if (magic != Ids::Magic)
        {
            error = ErrorCode::InvalidFileType;
            return false;
        }

        if (versionMajor != Header::MajorVersion)
        {
            error = ErrorCode::MajorVersionMismatch;
            return false;
        }

        if (versionMinor > Header::MinorVersion)
        {
            error = ErrorCode::MinorVersionMismatch;
            return false;
        }

        if (totalBytes < sizeof(Header))
        {
            error = ErrorCode::InvalidFileSize;
            return false;
        }

        if (blockCount > MaxBlocks)
        {
            error = ErrorCode::TooManyBlocks;
            return false;
        }

        if (blocksOffset != (sizeof(Header) + (blockCount * sizeof(BlockDescriptor))))
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

    bool LitlMesh::parse(std::span<std::byte const> data, LitlMesh& file, ErrorCode& error) noexcept
    {
        LitlMesh parsed{};
        error = ErrorCode::None;

        if (data.size() < sizeof(Header))
        {
            error = ErrorCode::InvalidFileSize;
            return false;
        }

        std::memcpy(&parsed.header, data.data(), sizeof(Header));

        if (!parsed.header.validate(error))
        {
            return false;
        }

        if (data.size() != static_cast<size_t>(parsed.header.totalBytes))
        {
            error = ErrorCode::InvalidFileSize;
            return false;
        }

        auto const contentHash = hashSubarray(data, parsed.header.blocksOffset, (parsed.header.totalBytes - parsed.header.blocksOffset));

        if (contentHash != parsed.header.contentHash)
        {
            error = ErrorCode::ContentHashMismatch;
            return false;
        }

        auto const descriptorBytes = size_t{ parsed.header.blockCount } * sizeof(BlockDescriptor);

        if (data.size() < (sizeof(Header) + descriptorBytes))
        {
            error = ErrorCode::InvalidFileSize;
            return false;
        }

        BinaryBlobReader reader({ data.data() + sizeof(Header), descriptorBytes });
        BlockDescriptor currDescriptor{};

        for (uint32_t i = 0u; i < parsed.header.blockCount; ++i)
        {
            if (!reader.read(currDescriptor))
            {
                error = ErrorCode::MissingBlockDescriptor;
            }

            if((currDescriptor.offset < parsed.header.blocksOffset) || (currDescriptor.offset > parsed.header.totalBytes))
            {
                error = ErrorCode::DescriptorBlockOutOfBounds;
                return false;
            }

            if (currDescriptor.blockBytes > (parsed.header.totalBytes - currDescriptor.offset))
            {
                error = ErrorCode::BlockSizeOutOfBounds;
                return false;
            }

            if ((uint64_t{ currDescriptor.elementBytes } * currDescriptor.elementCount) != currDescriptor.blockBytes)
            {
                error = ErrorCode::BlockSizeMismatch;
                return false;
            }

            parsed.descriptors[i] = currDescriptor;
        }

        parsed.descriptorCount = parsed.header.blockCount;
        parsed.data = data;
        file = parsed;
        return true;
    }

    bool LitlMesh::serialize(GeoMesh const& mesh, std::vector<std::byte>& data, ErrorCode& error) noexcept
    {
        error = ErrorCode::None;
        // ... todo ...
        return false;
    }

    bool LitlMesh::deserialize(GeoMesh& mesh, ErrorCode& error) const noexcept
    {
        error = ErrorCode::None;
        // ... todo ...
        return false;
    }

    std::optional<LitlMesh::Block> LitlMesh::find(std::array<char, 4> id) const noexcept
    {
        // ... todo ...
        return std::nullopt;
    }
}