#ifndef LITL_CORE_MATH_GEOMETRY_TOOLS_MESH_ORIENTATION_H__
#define LITL_CORE_MATH_GEOMETRY_TOOLS_MESH_ORIENTATION_H__

#include <cstdint>
#include <span>

#include "litl-core/math/geometry/vertex.hpp"

namespace litl::core
{
    struct MeshOrientationResult
    {
        uint32_t componentCount = 0u;
        uint32_t flippedFaces = 0u;
        uint32_t boundaryEdges = 0u;
        uint32_t nonManifoldEdges = 0u;
        bool nonOrientable = false;
    };

    /// <summary>
    /// Attempts to adjust the indices of a mesh such that the winding order is clockwise - which matches the internal winding used by LITL.
    /// </summary>
    [[nodiscard]] MeshOrientationResult orientateMesh(std::span<Vertex const> vertices, std::span<uint32_t> indices) noexcept;
}

#endif