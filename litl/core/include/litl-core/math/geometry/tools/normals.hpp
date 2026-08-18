#ifndef LITL_MATH_GEOMETRY_TOOLS_MESH_NORMALS_H__
#define LITL_MATH_GEOMETRY_TOOLS_MESH_NORMALS_H__

#include <cstdint>
#include <span>

#include "litl-core/math/geometry/vertex.hpp"

namespace litl
{
    /// <summary>
    /// Given the three positions comprising a face (in clockwise order), returns the face normal.
    /// Internally, LITL defaults to clockwise winding due to its left-handed coordinate system.
    /// </summary>
    [[nodiscard]] constexpr vec3 faceNormal(vec3 a, vec3 b, vec3 c) noexcept
    {
        return cross(c - a, b - a).normalized();
    }

    /// <summary>
    /// Given the three positions comprising a face (in clockwise order), returns the scaled face normal.
    /// The scaled, unnormalized, value is typically used for area weighting, degeneracy tests, etc.
    /// 
    /// Internally, LITL defaults to clockwise winding due to its left-handed coordinate system.
    /// </summary>
    [[nodiscard]] constexpr vec3 faceNormalScaled(vec3 a, vec3 b, vec3 c) noexcept
    {
        return cross(c - a, b - a);
    }

    /// <summary>
    /// Given the three positions comprising a face (in counter-clockwise order), returns the face normal.
    /// </summary>
    [[nodiscard]] constexpr vec3 faceNormalCCW(vec3 a, vec3 b, vec3 c) noexcept
    {
        return cross(b - a, c - a).normalized();
    }

    /// <summary>
    /// Given the three positions comprising a face (in counter-clockwise order), returns the scaled face normal.
    /// The scaled, unnormalized, value is typically used for area weighting, degeneracy tests, etc.
    /// </summary>
    [[nodiscard]] constexpr vec3 faceNormalScaledCCW(vec3 a, vec3 b, vec3 c) noexcept
    {
        return cross(b - a, c - a);
    }

    /// <summary>
    /// Given a mesh defined by its verticies and indices, recalculates the normals for each vertex.
    /// This function expects the mesh to be fully triangulated (all faces composed of 3 points) and 
    /// for all face indices to be ordered in clockwise winding.
    /// </summary>
    void calculateMeshNormals(std::span<Vertex> vertices, std::span<uint32_t> indices) noexcept;
}

#endif