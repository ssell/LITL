#ifndef LITL_MATH_GEOMETRY_MESH_H__
#define LITL_MATH_GEOMETRY_MESH_H__

#include <array>
#include <vector>

#include "litl-core/math/geometry/vertex.hpp"

namespace litl
{
    /// <summary>
    /// A geometric mesh, which is the intermediate data representation of a full mesh object.
    /// </summary>
    struct GeoMesh
    {
        /// <summary>
        /// Binary file representation of a GeoMesh.
        /// </summary>
        struct File
        {
            struct Header
            {
                uint32_t faceBlockStart{ 0u };
                uint32_t vertexBlockStart{ 0u };
                uint32_t indexBlockStart{ 0u };
            };

            struct FaceBlock
            {
                uint32_t blockSize{ sizeof(FaceBlock) };
                uint32_t faceSize{ static_cast<uint32_t>(sizeof(uint32_t)) };
                uint32_t faceCount{ 0u };

                // ... faces counts ...
            };

            struct VertexBlock
            {
                uint32_t blockSize{ sizeof(VertexBlock) };
                uint32_t vertexSize{ static_cast<uint32_t>(sizeof(Vertex)) };
                uint32_t vertexCount{ 0u };

                // ... vertices ...
            };

            struct IndexBlock
            {
                uint32_t blockSize{ sizeof(IndexBlock) };
                uint32_t indexSize{ static_cast<uint32_t>(sizeof(uint32_t)) };
                uint32_t indexCount{ 0u };

                // ... indices ...
            };

            Header header{};
            FaceBlock faces{};
            VertexBlock vertices{};
            IndexBlock indices{};
        };

        /// <summary>
        /// Recalculates and modifies the mesh in-place such that the "faceIndexCount" for every face is 3.
        /// </summary>
        void triangulate() noexcept;

        /// <summary>
        /// Each index is a face, and the value is the number of vertices in the face.
        /// For example, a mesh composed of 3 triangles and 1 quad may look like [3,3,3,4].
        /// </summary>
        std::vector<uint32_t> faceIndexCount;

        /// <summary>
        /// The distinct unique vertices within the mask. An individual vertex may be reused among multiple faces.
        /// </summary>
        std::vector<Vertex> vertices;

        /// <summary>
        /// The indices comprising each face of the mesh. The number of indices per face is stored in the faceIndexCount vector.
        /// </summary>
        std::vector<uint32_t> indices;
    };
}

#endif