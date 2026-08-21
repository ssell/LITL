#ifndef LITL_MATH_GEOMETRY_TOOLS_MESH_NORMALS_H__
#define LITL_MATH_GEOMETRY_TOOLS_MESH_NORMALS_H__

#include <cstdint>
#include <span>

#include "litl-core/math/geometry/vertex.hpp"

namespace litl
{
    /// <summary>
    /// Given a mesh defined by its verticies and indices, recalculates the normals for each vertex.
    /// This function expects the mesh to be fully triangulated (all faces composed of 3 points) and 
    /// for all face indices to be ordered in clockwise winding.
    /// </summary>
    void calculateMeshNormals(std::span<Vertex> vertices, std::span<uint32_t> indices) noexcept;
}

#endif