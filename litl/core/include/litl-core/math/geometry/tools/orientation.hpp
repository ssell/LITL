#ifndef LITL_MATH_GEOMETRY_TOOLS_MESH_ORIENTATION_H__
#define LITL_MATH_GEOMETRY_TOOLS_MESH_ORIENTATION_H__

#include <cstdint>
#include <span>

#include "litl-core/math/geometry/vertex.hpp"

namespace litl
{
    struct MeshOrientationReport
    {
        /// <summary>
        /// The number of components comprising the mesh.
        /// </summary>
        uint32_t componentCount = 0u;

        /// <summary>
        /// The number of faces that were flipped to be clockwise.
        /// </summary>
        uint32_t flippedFaces = 0u;

        /// <summary>
        /// The number of edges that do not border another face.
        /// </summary>
        uint32_t boundaryEdges = 0u;

        /// <summary>
        /// The number of edges that are not a proper manifold edge connecting to faces.
        /// </summary>
        uint32_t nonManifoldEdges = 0u;

        /// <summary>
        /// True if the mesh can not be orientated - for example a mobius strip.
        /// </summary>
        bool nonOrientable = false;
    };

    /// <summary>
    /// Attempts to adjust the indices of a mesh such that the winding order is clockwise - which matches the internal winding used by LITL.
    /// </summary>
    [[nodiscard]] MeshOrientationReport orientateMesh(std::span<Vertex const> vertices, std::span<uint32_t> indices) noexcept;
}

#endif