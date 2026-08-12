#include <cstring>

#include "litl-core/hash.hpp"
#include "litl-core/containers/common.hpp"
#include "litl-core/formats/litlmesh.hpp"
#include "litl-core/formats/binaryBlobReader.hpp"

namespace litl
{
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

    void serializeHeaderBounds(GeoMesh const& mesh, LitlMesh::Header& header) noexcept
    {
        vec3 meshMinPoint{};
        vec3 meshMaxPoint{};

        mesh.getMinMaxPoints(meshMinPoint, meshMaxPoint);

        header.boundsMin = meshMinPoint.toArray();
        header.boundsMax = meshMaxPoint.toArray();
    }

    template<typename T>
    void serializeBlock(LitlMesh::BlockDescriptor& descriptor, LitlMesh::Block& block, size_t& runningBlockOffset, LitlMesh::BlockIdType const& id, std::span<T const> elements, uint32_t flags) noexcept
    {
        descriptor.blockId = id;
        descriptor.elementBytes = static_cast<uint32_t>(sizeof(T));
        descriptor.elementCount = static_cast<uint32_t>(elements.size());
        descriptor.blockOffset = static_cast<uint32_t>(runningBlockOffset);
        descriptor.blockBytes = static_cast<uint32_t>(sizeof(LitlMesh::Block) + (descriptor.elementBytes * descriptor.elementCount));
        descriptor.flags = flags;

        block.blockId = descriptor.blockId;
        block.elementBytes = descriptor.elementBytes;
        block.elementCount = descriptor.elementCount;
        block.bytes = as_byte_span(elements);

        runningBlockOffset += descriptor.blockBytes;
    }

    void runningCopy(void const* from, std::byte* to, size_t size, size_t& runningOffset) noexcept
    {
        std::memcpy(to + runningOffset, from, size);
        runningOffset += size;
    }

    bool LitlMesh::serialize(GeoMesh const& mesh, std::vector<std::byte>& data, ErrorCode& error) noexcept
    {
        error = ErrorCode::None;

        LitlMesh litlMesh{};

        // ---------------------------------------------------------------------------------
        // Populate Header (most of it)

        litlMesh.header.magic = Ids::Magic;
        litlMesh.header.versionMajor = Header::MajorVersion;
        litlMesh.header.versionMinor = Header::MinorVersion;
        litlMesh.header.contentHash = 0ull;        // to be computed later  (TODO)
        litlMesh.header.totalBytes = 0ull;         // to be computed later  (TODO)
        litlMesh.header.blockCount = 2u;
        litlMesh.header.blocksOffset = static_cast<uint32_t>(sizeof(Header) + (sizeof(BlockDescriptor) * litlMesh.header.blockCount));
        litlMesh.header.flags = 0u;
        litlMesh.header.reserved = 0u;

        serializeHeaderBounds(mesh, litlMesh.header);

        // ---------------------------------------------------------------------------------
        // Populate BlockDescriptors and Blocks

        std::array<Block, MaxBlocks> blocks{};

        BlockDescriptor& vertexBlockDescriptor = litlMesh.descriptors[0];
        BlockDescriptor& indexBlockDescriptor = litlMesh.descriptors[1];

        Block& vertexBlock = blocks[0];
        Block& indexBlock = blocks[1];

        size_t runningOffset = litlMesh.header.blocksOffset;

        serializeBlock<Vertex>(vertexBlockDescriptor, vertexBlock, runningOffset, Ids::Vertices, mesh.vertices, 0u);
        serializeBlock<uint32_t>(indexBlockDescriptor, indexBlock, runningOffset, Ids::Indices, mesh.indices, 0u);

        litlMesh.header.totalBytes = runningOffset;

        // ---------------------------------------------------------------------------------
        // Copy Content to Temporary Buffer

        std::vector<std::byte> contentBytes(static_cast<size_t>(litlMesh.header.totalBytes - sizeof(Header)));
        std::fill(contentBytes.begin(), contentBytes.end(), std::byte(0));

        runningOffset = 0;

        for (auto i = 0u; i < litlMesh.header.blockCount; ++i)
        {
            runningCopy(&litlMesh.descriptors[i], contentBytes.data(), sizeof(BlockDescriptor), runningOffset);
        }

        for (auto i = 0u; i < litlMesh.header.blockCount; ++i)
        {
            runningCopy(&blocks[i], contentBytes.data(), sizeof(Block), runningOffset);
            runningCopy(blocks[i].bytes.data(), contentBytes.data(), blocks[i].bytes.size(), runningOffset);
        }

        // ---------------------------------------------------------------------------------
        // Calculate Content Hash

        litlMesh.header.contentHash = hashArray(std::span<std::byte const>(contentBytes));

        // ---------------------------------------------------------------------------------
        // Copy to Final Destination

        if (litlMesh.header.totalBytes != static_cast<uint32_t>(sizeof(Header) + contentBytes.size()))
        {
            error = ErrorCode::SerializationSizeMismatch;
            return false;
        }

        data.resize(litlMesh.header.totalBytes);
        std::fill(data.begin(), data.end(), std::byte(0));

        std::memcpy(data.data(), &litlMesh.header, sizeof(Header));
        std::memcpy(data.data() + sizeof(Header), contentBytes.data(), contentBytes.size());

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

    std::optional<LitlMesh::Block> LitlMesh::find(std::array<char, 4> id) const noexcept
    {
        for (auto& descriptor : descriptors)
        {
            if (descriptor.blockId == id)
            {
                if (static_cast<size_t>(descriptor.blockOffset + descriptor.blockBytes) < data.size())
                {
                    LitlMesh::Block dataBlock{};
                    std::memcpy(&dataBlock, data.data(), descriptor.blockBytes);
                    return dataBlock;
                }
            }
        }
        return std::nullopt;
    }
}