#include <algorithm>
#include <cstring>

#include "litl-core/hash.hpp"
#include "litl-core/containers/common.hpp"
#include "litl-core/formats/litlmesh.hpp"

namespace litl
{
    namespace
    {
        void serializeBounds(GeoMesh const& mesh, std::array<float, 6>& boundsMinMaxPoints) noexcept
        {
            const auto& bounds = mesh.getBounds();

            boundsMinMaxPoints[0] = bounds.min.x();
            boundsMinMaxPoints[1] = bounds.min.y();
            boundsMinMaxPoints[2] = bounds.min.z();

            boundsMinMaxPoints[3] = bounds.max.x();
            boundsMinMaxPoints[4] = bounds.max.y();
            boundsMinMaxPoints[5] = bounds.max.z();
        }

        [[nodiscard]] bool shouldSetAllTrianglesFlag(GeoMesh const& mesh) noexcept
        {
            for (auto faceIndexCount : mesh.getFaceIndexCounts())
            {
                if (faceIndexCount != 3u)
                {
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]] LitlMeshFlag determineFlags(GeoMesh const& mesh) noexcept
        {
            LitlMeshFlag flag = LitlMeshFlagBits::None;

            if (shouldSetAllTrianglesFlag(mesh)) { flag |= LitlMeshFlagBits::AllTriangles; }
            // ... todo add others ...

            return flag;
        }
    }

    // -------------------------------------------------------------------------------------
    // Serialization
    // -------------------------------------------------------------------------------------

    bool LitlMesh::serialize(GeoMesh const& mesh, std::vector<std::byte>& data, ErrorCode& error) noexcept
    {
        /*
         * TODO: In the future, serialize can likely be abstracted out a lot more.
         * It can potentially be condensed to the expected blockDataTable and fed to a common serialize in BinaryBlockFile.
         * But it is best to do that after we have another file format (such as a potential texture format) that is built on BinaryBlockFile.
         * A potential abstraction would be a block builder:
         * 
         *     addBlock(...);
         *     finalize(...);
         */

        error = ErrorCode::None;

        if (mesh.vertexCount() == 0 || mesh.indexCount() == 0 || mesh.faceCount() == 0)
        {
            error = ErrorCode::SourceMeshEmpty;
            return false;
        }

        LitlMesh litlMesh{};
        const LitlMeshFlag flags = determineFlags(mesh);
        StringMap stringMap{};

        // Even if AllTriangles is set, the appearance of the block is required. This leads to 16 bytes of bloat, but consistent binary structure regardless of flags.
        std::array<uint32_t, 0> decoyFaces{};
        auto faceByteSpan = has_any(flags, LitlMeshFlagBits::AllTriangles) ? as_byte_span(decoyFaces) : as_byte_span(mesh.getFaceIndexCounts());

        std::array<float, 6> boundsMinMaxPoints{};
        serializeBounds(mesh, boundsMinMaxPoints);

        std::vector<BlockDataDescriptor> blockDataTable; blockDataTable.reserve(MaxBlocks);
        litlMesh.addDefaultBlockDescriptors(blockDataTable);

        if (!litlMesh.addDataBlockDescriptor(blockDataTable, BinaryBlockFile::DefaultBlocks::DefaultBlocksCount + 0, BlockIds::Bounds, sizeof(float), as_byte_span(boundsMinMaxPoints), error) ||
            !litlMesh.addDataBlockDescriptor(blockDataTable, BinaryBlockFile::DefaultBlocks::DefaultBlocksCount + 1, BlockIds::Vertices, sizeof(Vertex), as_byte_span(mesh.getVertices()), error) ||
            !litlMesh.addDataBlockDescriptor(blockDataTable, BinaryBlockFile::DefaultBlocks::DefaultBlocksCount + 2, BlockIds::Indices, sizeof(uint32_t), as_byte_span(mesh.getIndices()), error) ||
            !litlMesh.addDataBlockDescriptor(blockDataTable, BinaryBlockFile::DefaultBlocks::DefaultBlocksCount + 3, BlockIds::Faces, sizeof(uint32_t), faceByteSpan, error))
        {
            // ... ErrorCode::TooManyBlocks set by addDataBlockDescriptor ...
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

        litlMesh.header.magic = Identity.magic;
        litlMesh.header.versionMajor = Identity.versionMajor;
        litlMesh.header.versionMinor = Identity.versionMinor;
        litlMesh.header.contentHash = 0ull;         // calculated further on
        litlMesh.header.totalBytes = 0u;            // calculated further on
        litlMesh.header.blockCount = static_cast<uint32_t>(blockDataTable.size());
        litlMesh.header.descriptorsOffset = sizeof(Header);
        litlMesh.header.blocksOffset = litlMesh.header.descriptorsOffset + (sizeof(BlockDescriptor) * litlMesh.header.blockCount);
        litlMesh.header.flags = static_cast<uint32_t>(flags);

        // ---------------------------------------------------------------------------------
        // Populate BlockDescriptors

        litlMesh.serializeDefaultBlocks(blockDataTable, stringMap);

        uint64_t runningOffset = litlMesh.header.blocksOffset;

        for (uint32_t i = 0; i < litlMesh.header.blockCount; ++i)
        {
            auto& blockData = blockDataTable[i];
            serializeBlock(blockData, runningOffset);
        }

        litlMesh.header.totalBytes = runningOffset;

        // ---------------------------------------------------------------------------------
        // Copy content to the provided data buffer

        serializeDataBuffer(litlMesh, blockDataTable, data);

        return true;
    }

    // -------------------------------------------------------------------------------------
    // Deserialization
    // -------------------------------------------------------------------------------------

    namespace
    {
        [[nodiscard]] bool deserializeFaceBlock(GeoMesh& mesh, std::span<uint32_t const> faces, std::span<uint32_t const> indices, LitlMeshFlag flags, BinaryBlockFile::ErrorCode& error) noexcept
        {
            const bool allTriangles = has_any(flags, LitlMeshFlagBits::AllTriangles);

            if (faces.empty() && !allTriangles)
            {
                error = BinaryBlockFile::ErrorCode::MissingFaceBlock;
                return false;
            }

            if (allTriangles)
            {
                mesh.setAllFaceIndexCounts(3u);
            }
            else
            {
                uint64_t sumFaceIndexCount = 0u;

                for (auto faceCount : faces)
                {
                    if (faceCount == 0u)
                    {
                        error = BinaryBlockFile::ErrorCode::ZeroFaceFound;
                        return false;
                    }

                    sumFaceIndexCount += faceCount;

                    if (sumFaceIndexCount > indices.size())
                    {
                        error = BinaryBlockFile::ErrorCode::InvalidFaceSum;
                        return false;
                    }
                }

                if (sumFaceIndexCount != indices.size())
                {
                    error = BinaryBlockFile::ErrorCode::InvalidFaceSum;
                    return false;
                }

                mesh.setFaceIndexCounts(faces);
            }

            return true;
        }
    }

    bool LitlMesh::deserialize(GeoMesh& mesh, ErrorCode& error) const noexcept
    {
        const LitlMeshFlag flags = static_cast<LitlMeshFlag>(header.flags);
        auto stringsBlock = find(DefaultBlocks::Strings);
        auto vertexBlock = find(BlockIds::Vertices);
        auto indexBlock = find(BlockIds::Indices);
        auto faceBlock = find(BlockIds::Faces);
        auto boundsBlock = find(BlockIds::Bounds);

        if (!stringsBlock.has_value()) { error = ErrorCode::MissingStringsBlock; return false; }
        if (!vertexBlock.has_value()) { error = ErrorCode::MissingVertexBlock; return false; }
        if (!indexBlock.has_value()) { error = ErrorCode::MissingIndexBlock; return false; }
        if (!faceBlock.has_value()) { error = ErrorCode::MissingFaceBlock; return false; }
        if (!boundsBlock.has_value()) { error = ErrorCode::MissingBoundsBlock; return false; }

        auto strings = stringsBlock.value().as<char const>(error).value_or({});
        auto vertices = vertexBlock.value().as<Vertex>(error).value_or({});
        auto indices = indexBlock.value().as<uint32_t>(error).value_or({});
        auto faces = faceBlock.value().as<uint32_t>(error).value_or({});
        auto bounds = boundsBlock.value().as<float>(error).value_or({});

        if (error != ErrorCode::None)
        {
            return false;
        }

        for (auto index : indices)
        {
            if (index >= vertices.size())
            {
                error = ErrorCode::InvalidIndexFound;
                return false;
            }
        }

        if (bounds.size() != 6ull)
        {
            error = ErrorCode::InvalidBoundsValues;
            return false;
        }

        mesh.setVertices(vertices);
        mesh.setIndices(indices);

        if (!deserializeFaceBlock(mesh, faces, indices, flags, error))
        {
            mesh.clear();
            return false;
        }

        mesh.setBoundsMinMax(vec3{ bounds[0], bounds[1], bounds[2] }, vec3{ bounds[3], bounds[4], bounds[5] });
        mesh.setWindingOrder(MeshWinding::Clockwise);       // ImportService ensures mesh orientation during import/export of a litlmesh

        return true;
    }
}