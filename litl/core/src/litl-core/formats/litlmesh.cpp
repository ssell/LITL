#include "litl-core/formats/litlmesh.hpp"
#include "litl-core/formats/binaryBlobReader.hpp"

namespace litl
{
    bool LitlMesh::Header::validate(ErrorCode& error) noexcept
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

        if (headerBytes != sizeof(Header))
        {
            error = ErrorCode::HeaderStructureMismatch;
            return false;
        }

        if (totalBytes < sizeof(LitlMesh))
        {
            error = ErrorCode::FileTooSmall;
            return false;
        }

        if (blockCount == 0u)
        {
            error = ErrorCode::MissingBlocks;
            return false;
        }

        if (blockCount > MaxBlocks)
        {
            error = ErrorCode::TooManyBlocks;
            return false;
        }

        return true;
    }

    bool LitlMesh::parse(std::span<std::byte const> data, LitlMesh& file, ErrorCode& error) noexcept
    {
        LitlMesh parsed{};
        error = ErrorCode::None;

        if (data.size() < sizeof(LitlMesh))
        {
            error = ErrorCode::InvalidFileSize;
            return false;
        }

        std::memcpy(&parsed.header, data.data(), sizeof(Header));

        if (!parsed.header.validate(error))
        {
            return false;
        }

        if (static_cast<uint32_t>(data.size()) != parsed.header.totalBytes)
        {
            error = ErrorCode::FileSizeMismatch;
            return false;
        }

        BinaryBlobReader reader({ data.data() + sizeof(Header), sizeof(BlockDescriptor) * MaxBlocks });
        BlockDescriptor currDescriptor{};
        uint32_t descriptorIndex = 0u;

        while (reader.read(currDescriptor) && (descriptorIndex++ < parsed.header.blockCount))
        {
            if ((currDescriptor.offset < parsed.header.headerBytes) || (currDescriptor.offset > parsed.header.totalBytes))
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

            memcpy(&currDescriptor, &parsed.descriptors[descriptorIndex], sizeof(BlockDescriptor));
        }

        parsed.data = data;
        file = parsed;
        return true;
    }

    bool LitlMesh::serialize(GeoMesh const& mesh, std::vector<std::byte>& data, ErrorCode& error) noexcept
    {
        error = ErrorCode::None;
        return false;
    }

    bool LitlMesh::deserialize(GeoMesh& mesh, ErrorCode& error) noexcept
    {
        error = ErrorCode::None;
        return false;
    }

    std::optional<LitlMesh::Block> LitlMesh::find(std::array<char, 4> id) const noexcept
    {
        return std::nullopt;
    }
}