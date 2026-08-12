#include <algorithm>
#include <cstring>
#include <limits>

#include "litl-core/hash.hpp"
#include "litl-core/formats/litlmesh.hpp"
#include "litl-core/formats/binaryBlobReader.hpp"

namespace litl
{
    namespace
    {
        void serializeHeaderBounds(GeoMesh const& mesh, LitlMesh::Header& header) noexcept
        {
            vec3 meshMinPoint{};
            vec3 meshMaxPoint{};

            mesh.getMinMaxPoints(meshMinPoint, meshMaxPoint);

            header.boundsMin = meshMinPoint.toArray();
            header.boundsMax = meshMaxPoint.toArray();
        }

        template<typename T>
        void serializeBlock(LitlMesh::BlockDescriptor& descriptor, size_t& runningBlockOffset, LitlMesh::BlockIdType const& id, std::span<T const> elements, uint32_t flags) noexcept
        {
            descriptor.blockId = id;
            descriptor.elementBytes = static_cast<uint32_t>(sizeof(T));
            descriptor.elementCount = static_cast<uint32_t>(elements.size());
            descriptor.blockOffset = static_cast<uint32_t>(runningBlockOffset);
            descriptor.blockBytes = static_cast<size_t>(descriptor.elementBytes) * descriptor.elementCount;
            descriptor.flags = flags;

            runningBlockOffset += descriptor.blockBytes;
        }

        void runningCopy(void const* from, std::byte* to, size_t size, size_t& runningOffset) noexcept
        {
            std::memcpy(to + runningOffset, from, size);
            runningOffset += size;
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

        // ---------------------------------------------------------------------------------
        // Populate Header (most of it)

        litlMesh.header.magic = Ids::Magic;
        litlMesh.header.versionMajor = Header::MajorVersion;
        litlMesh.header.versionMinor = Header::MinorVersion;
        litlMesh.header.contentHash = 0ull;        // to be computed later  (TODO)
        litlMesh.header.totalBytes = 0ull;         // to be computed later  (TODO)
        litlMesh.header.blockCount = 3u;
        litlMesh.header.blocksOffset = static_cast<uint32_t>(sizeof(Header) + (sizeof(BlockDescriptor) * litlMesh.header.blockCount));
        litlMesh.header.flags = 0u;
        litlMesh.header.reserved = 0u;

        serializeHeaderBounds(mesh, litlMesh.header);

        // ---------------------------------------------------------------------------------
        // Populate BlockDescriptors and Blocks

        BlockDescriptor& vertexBlockDescriptor = litlMesh.descriptors[0];
        BlockDescriptor& indexBlockDescriptor = litlMesh.descriptors[1];
        BlockDescriptor& faceBlockDescriptor = litlMesh.descriptors[2];

        size_t runningOffset = litlMesh.header.blocksOffset;

        serializeBlock<Vertex>(vertexBlockDescriptor, runningOffset, Ids::Vertices, mesh.vertices, 0u);
        serializeBlock<uint32_t>(indexBlockDescriptor, runningOffset, Ids::Indices, mesh.indices, 0u);
        serializeBlock<uint32_t>(indexBlockDescriptor, runningOffset, Ids::Faces, mesh.faceIndexCount, 0u);

        if (runningOffset > size_t{ std::numeric_limits<uint32_t>::max() })
        {
            error = ErrorCode::ContentTooLarge;
            return false;
        }

        litlMesh.header.totalBytes = static_cast<uint32_t>(runningOffset);

        // ---------------------------------------------------------------------------------
        // Copy Content to Temporary Buffer

        data.resize(litlMesh.header.totalBytes);
        std::fill(data.begin(), data.end(), std::byte(0));

        runningOffset = sizeof(Header);

        runningCopy(&vertexBlockDescriptor, data.data(), sizeof(BlockDescriptor), runningOffset);
        runningCopy(&indexBlockDescriptor, data.data(), sizeof(BlockDescriptor), runningOffset);
        runningCopy(&faceBlockDescriptor, data.data(), sizeof(BlockDescriptor), runningOffset);

        runningCopy(mesh.vertices.data(), data.data(), vertexBlockDescriptor.blockBytes, runningOffset);
        runningCopy(mesh.indices.data(), data.data(), indexBlockDescriptor.blockBytes, runningOffset);
        runningCopy(mesh.faceIndexCount.data(), data.data(), faceBlockDescriptor.blockBytes, runningOffset);

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