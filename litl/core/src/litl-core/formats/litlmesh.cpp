#include <algorithm>
#include <cstring>
#include <limits>

#include "litl-core/hash.hpp"
#include "litl-core/math/common.hpp"
#include "litl-core/containers/common.hpp"
#include "litl-core/formats/litlmesh.hpp"
#include "litl-core/formats/binaryBlobReader.hpp"

namespace litl
{
    namespace
    {
        struct BlockData
        {
            LitlMesh::BlockDescriptor* descriptor;
            LitlMesh::BlockIdType id;
            size_t elementSize;
            std::span<std::byte const> data;
        };

        void serializeHeaderBounds(GeoMesh const& mesh, LitlMesh::Header& header) noexcept
        {
            vec3 meshMinPoint{};
            vec3 meshMaxPoint{};

            mesh.getMinMaxPoints(meshMinPoint, meshMaxPoint);

            header.boundsMin = meshMinPoint.toArray();
            header.boundsMax = meshMaxPoint.toArray();
        }

        void serializeBlock(BlockData& data, size_t& runningBlockOffset) noexcept
        {
            // Ensure our offsets remain a multiple of 16
            runningBlockOffset = alignMemoryOffsetUp(runningBlockOffset, 16);

            data.descriptor->blockId = data.id;
            data.descriptor->elementBytes = static_cast<uint32_t>(data.elementSize);
            data.descriptor->elementCount = static_cast<uint32_t>(data.data.size() / data.elementSize);
            data.descriptor->blockOffset = static_cast<uint32_t>(runningBlockOffset);
            data.descriptor->blockBytes = static_cast<uint32_t>(data.data.size());
            data.descriptor->flags = 0u;

            runningBlockOffset += data.descriptor->blockBytes;
        }
    }

    // -------------------------------------------------------------------------------------
    // Parsing
    // -------------------------------------------------------------------------------------

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

        auto const contentHash = hashSubarray(data, sizeof(Header), (parsed.header.totalBytes - sizeof(Header)));

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
        uint32_t prevBlockEnd = 0u;

        for (uint32_t i = 0u; i < parsed.header.blockCount; ++i)
        {
            if (!reader.read(currDescriptor))
            {
                error = ErrorCode::MissingBlockDescriptor;
                return false;
            }

            if((currDescriptor.blockOffset < parsed.header.blocksOffset) || (currDescriptor.blockOffset > parsed.header.totalBytes))
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

            parsed.descriptors[i] = currDescriptor;

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

            prevBlockEnd = currDescriptor.blockOffset + currDescriptor.blockBytes;
        }

        parsed.data = data;
        file = parsed;
        return true;
    }

    // -------------------------------------------------------------------------------------
    // Serialization
    // -------------------------------------------------------------------------------------

    bool LitlMesh::serialize(GeoMesh const& mesh, std::vector<std::byte>& data, ErrorCode& error) noexcept
    {
        error = ErrorCode::None;

        if (mesh.vertices.empty() || mesh.indices.empty() || mesh.faceIndexCount.empty())
        {
            error = ErrorCode::SourceMeshEmpty;
            return false;
        }

        LitlMesh litlMesh{};

        std::array<BlockData, 3> blockDataTable {
            BlockData { &litlMesh.descriptors[0], Ids::Vertices, sizeof(Vertex), as_byte_span(mesh.vertices) },
            BlockData { &litlMesh.descriptors[1], Ids::Indices, sizeof(uint32_t), as_byte_span(mesh.indices) },
            BlockData { &litlMesh.descriptors[2], Ids::Faces, sizeof(uint32_t), as_byte_span(mesh.faceIndexCount) }
        };

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

        litlMesh.header.magic = Ids::Magic;
        litlMesh.header.versionMajor = Header::MajorVersion;
        litlMesh.header.versionMinor = Header::MinorVersion;
        litlMesh.header.contentHash = 0ull;         // calculated further on
        litlMesh.header.totalBytes = 0u;            // calculated further on
        litlMesh.header.blockCount = static_cast<uint32_t>(blockDataTable.size());
        litlMesh.header.blocksOffset = static_cast<uint32_t>(sizeof(Header) + (sizeof(BlockDescriptor) * litlMesh.header.blockCount));
        litlMesh.header.flags = 0u;                 // currently unused
        litlMesh.header.reserved = 0u;              // intentional padding

        serializeHeaderBounds(mesh, litlMesh.header);

        // ---------------------------------------------------------------------------------
        // Populate BlockDescriptors

        size_t runningOffset = litlMesh.header.blocksOffset;

        for (uint32_t i = 0; i < litlMesh.header.blockCount; ++i)
        {
            serializeBlock(blockDataTable[i], runningOffset);
        }

        if (runningOffset > size_t{ std::numeric_limits<uint32_t>::max() })
        {
            error = ErrorCode::ContentTooLarge;
            return false;
        }

        litlMesh.header.totalBytes = static_cast<uint32_t>(runningOffset);

        // ---------------------------------------------------------------------------------
        // Copy content to the provided data buffer

        data.resize(litlMesh.header.totalBytes);
        std::fill(data.begin(), data.end(), std::byte(0));

        for (size_t i = 0ull; i < blockDataTable.size(); ++i)
        {
            std::memcpy(data.data() + sizeof(Header) + (sizeof(BlockDescriptor) * i), blockDataTable[i].descriptor, sizeof(BlockDescriptor));
        }

        for (auto& blockData : blockDataTable)
        {
            std::memcpy(data.data() + blockData.descriptor->blockOffset, blockData.data.data(), blockData.data.size());
        }

        litlMesh.header.contentHash = hashSubarray(std::span<std::byte const>(data), sizeof(Header), data.size() - sizeof(Header));

        std::memcpy(data.data(), &litlMesh.header, sizeof(Header));

        return true;
    }

    // -------------------------------------------------------------------------------------
    // Deserialization
    // -------------------------------------------------------------------------------------

    bool LitlMesh::deserialize(GeoMesh& mesh, ErrorCode& error) const noexcept
    {
        error = ErrorCode::None;
        // ... todo ...
        return false;
    }

    // -------------------------------------------------------------------------------------
    // Utility
    // -------------------------------------------------------------------------------------

    std::optional<LitlMesh::Block> LitlMesh::find(BlockIdType id) const noexcept
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