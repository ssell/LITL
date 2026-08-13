#include <algorithm>
#include <cstring>

#include "litl-core/hash.hpp"
#include "litl-core/containers/common.hpp"
#include "litl-core/formats/litlmesh.hpp"

namespace litl
{
    namespace
    {
        struct LitlMeshIds
        {
            /// <summary>
            /// Id for a block of vertex data - VRTX
            /// </summary>
            static constexpr BinaryBlockFile::BlockIdType Vertices{ 'V', 'R', 'T', 'X' };

            /// <summary>
            /// Id for a block of index data - INDX
            /// </summary>
            static constexpr BinaryBlockFile::BlockIdType Indices{ 'I', 'N', 'D', 'X' };

            /// <summary>
            /// Id for a block of face index count data - FACE
            /// </summary>
            static constexpr BinaryBlockFile::BlockIdType Faces{ 'F', 'A', 'C', 'E' };

            /// <summary>
            /// Id for a block that describes the min/max points of a mesh AABB bounds - BNDS.
            /// </summary>
            static constexpr BinaryBlockFile::BlockIdType Bounds{ 'B', 'N', 'D', 'S' };
        };

        void serializeHeaderBounds(GeoMesh const& mesh, std::array<float, 6>& boundsMinMaxPoints) noexcept
        {
            const auto& bounds = mesh.getBounds();

            boundsMinMaxPoints[0] = bounds.min.x();
            boundsMinMaxPoints[1] = bounds.min.y();
            boundsMinMaxPoints[2] = bounds.min.z();

            boundsMinMaxPoints[3] = bounds.max.x();
            boundsMinMaxPoints[4] = bounds.max.y();
            boundsMinMaxPoints[5] = bounds.max.z();
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

        std::array<float, 6> boundsMinMaxPoints{};
        serializeHeaderBounds(mesh, boundsMinMaxPoints);

        std::array<BlockDataDescriptor, 4> blockDataTable {
            BlockDataDescriptor { &litlMesh.descriptors[0], LitlMeshIds::Bounds, sizeof(float), as_byte_span(boundsMinMaxPoints) },
            BlockDataDescriptor { &litlMesh.descriptors[1], LitlMeshIds::Vertices, sizeof(Vertex), as_byte_span(mesh.getVertices()) },
            BlockDataDescriptor { &litlMesh.descriptors[2], LitlMeshIds::Indices, sizeof(uint32_t), as_byte_span(mesh.getIndices()) },
            BlockDataDescriptor { &litlMesh.descriptors[3], LitlMeshIds::Faces, sizeof(uint32_t), as_byte_span(mesh.getFaceIndexCounts()) },
        };

        static_assert(std::tuple_size_v<decltype(blockDataTable)> <= MaxBlocks);

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

        litlMesh.header.magic = Magic;
        litlMesh.header.versionMajor = Header::MajorVersion;
        litlMesh.header.versionMinor = Header::MinorVersion;
        litlMesh.header.contentHash = 0ull;         // calculated further on
        litlMesh.header.totalBytes = 0u;            // calculated further on
        litlMesh.header.blockCount = static_cast<uint32_t>(blockDataTable.size());
        litlMesh.header.descriptorsOffset = sizeof(Header);
        litlMesh.header.blocksOffset = litlMesh.header.descriptorsOffset + (sizeof(BlockDescriptor) * litlMesh.header.blockCount);
        litlMesh.header.flags = 0u;                 // currently unused

        // ---------------------------------------------------------------------------------
        // Populate BlockDescriptors

        uint64_t runningOffset = litlMesh.header.blocksOffset;

        for (uint32_t i = 0; i < litlMesh.header.blockCount; ++i)
        {
            serializeBlock(blockDataTable[i], runningOffset);
        }

        litlMesh.header.totalBytes = runningOffset;

        // ---------------------------------------------------------------------------------
        // Copy content to the provided data buffer

        data.resize(litlMesh.header.totalBytes);
        std::fill(data.begin(), data.end(), std::byte(0));

        for (size_t i = 0ull; i < blockDataTable.size(); ++i)
        {
            std::memcpy(data.data() + litlMesh.header.descriptorsOffset + (sizeof(BlockDescriptor) * i), blockDataTable[i].descriptor, sizeof(BlockDescriptor));
        }

        for (auto& blockData : blockDataTable)
        {
            std::memcpy(data.data() + blockData.descriptor->blockOffset, blockData.data.data(), blockData.data.size());
        }

        litlMesh.header.contentHash = calculateContentHash(std::span<std::byte const>(data), litlMesh.header);

        std::memcpy(data.data(), &litlMesh.header, sizeof(Header));

        return true;
    }

    // -------------------------------------------------------------------------------------
    // Deserialization
    // -------------------------------------------------------------------------------------

    bool LitlMesh::deserialize(GeoMesh& mesh, ErrorCode& error) const noexcept
    {
        error = ErrorCode::None;

        auto vertexBlock = find(LitlMeshIds::Vertices);
        auto indexBlock = find(LitlMeshIds::Indices);
        auto faceBlock = find(LitlMeshIds::Faces);
        auto boundsBlock = find(LitlMeshIds::Bounds);

        if (!vertexBlock.has_value())
        {
            error = ErrorCode::MissingVertexBlock;
            return false;
        }

        if (!indexBlock.has_value())
        {
            error = ErrorCode::MissingIndexBlock;
            return false;
        }

        if (!faceBlock.has_value())
        {
            error = ErrorCode::MissingFaceBlock;
            return false;
        }

        if (!boundsBlock.has_value())
        {
            error = ErrorCode::MissingBoundsBlock;
            return false;
        }

        auto vertices = vertexBlock.value().as<Vertex>(error);

        if (!vertices.has_value())
        {
            return false;
        }

        auto indices = indexBlock.value().as<uint32_t>(error);

        if (!indices.has_value())
        {
            return false;
        }

        auto faces = faceBlock.value().as<uint32_t>(error);

        if (!faces.has_value())
        {
            return false;
        }

        auto bounds = boundsBlock.value().as<float>(error);

        if (!bounds.has_value())
        {
            return false;
        }

        for (auto index : indices.value())
        {
            if (index >= vertices->size())
            {
                error = ErrorCode::InvalidIndexFound;
                return false;
            }
        }

        uint64_t sumFaceIndexCount = 0u;

        for (auto faceCount : faces.value())
        {
            if (faceCount == 0u)
            {
                error = ErrorCode::ZeroFaceFound;
                return false;
            }

            sumFaceIndexCount += faceCount;

            if (sumFaceIndexCount > indices->size())
            {
                error = ErrorCode::InvalidFaceSum;
                return false;
            }
        }

        if (sumFaceIndexCount != indices->size())
        {
            error = ErrorCode::InvalidFaceSum;
            return false;
        }

        if (bounds->size() != 6ull)
        {
            error = ErrorCode::InvalidBoundsValues;
            return false;
        }

        mesh.setVertices(vertices.value());
        mesh.setIndices(indices.value());
        mesh.setFaceIndexCounts(faces.value());
        mesh.setBoundsMinMax(vec3{ bounds.value()[0], bounds.value()[1], bounds.value()[2] }, vec3{ bounds.value()[3], bounds.value()[4], bounds.value()[5] });

        return true;
    }
}