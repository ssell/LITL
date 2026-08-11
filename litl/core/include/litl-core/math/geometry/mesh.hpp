#ifndef LITL_MATH_GEOMETRY_MESH_H__
#define LITL_MATH_GEOMETRY_MESH_H__

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