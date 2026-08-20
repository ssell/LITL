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
        return cross(c - a, b - a);    // face area = (length / 2); area near 0 = degenerate triangle
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
        return cross(b - a, c - a);    // face area = (length / 2); area near 0 = degenerate triangle
    }
    
    /// <summary>
    /// Calculates the scaled normal for a set of vertex positions defining an n-gon in counter-clockwise order.
    /// The scaled, unnormalized, value is typically used for area weighting, degeneracy tests, etc.
    /// </summary>
    [[nodiscard]] constexpr vec3 ngonFaceNormalScaledCCW(std::span<vec3 const> face) noexcept
    {
        // Using Newell's method
        vec3 normal{};

        if (face.size() >= 3)
        {
            for (size_t i = 0, j = face.size() - 1; i < face.size(); j = i++)       // note that j starts at the end, and then follows one behind
            {
                const vec3& curr = face[j];
                const vec3& next = face[i];

                normal.x() += (curr.y() - next.y()) * (curr.z() + next.z());        // note this looks like the cross product, but its not.
                normal.y() += (curr.z() - next.z()) * (curr.x() + next.x());
                normal.z() += (curr.x() - next.x()) * (curr.y() + next.y());
            }
        }

        return normal;
    }

    /// <summary>
    /// Calculates the normal for a set of vertex positions defining an n-gon in counter-clockwise order.
    /// </summary>
    [[nodiscard]] constexpr vec3 ngonFaceNormalCCW(std::span<vec3 const> face) noexcept
    {
        return ngonFaceNormalScaledCCW(face).normalized();
    }

    /// <summary>
    /// Calculates the scaled normal for a set of vertex positions defining an n-gon in clockwise order.
    /// The scaled, unnormalized, value is typically used for area weighting, degeneracy tests, etc.
    /// </summary>
    [[nodiscard]] constexpr vec3 ngonFaceNormalScaled(std::span<vec3 const> face) noexcept
    {
        // Negate the accumulated normal to flip the winding to clockwise
        return -ngonFaceNormalScaledCCW(face);
    }

    /// <summary>
    /// Calculates the normal for a set of vertex positions defining an n-gon in counter-clockwise order.
    /// </summary>
    [[nodiscard]] constexpr vec3 ngonFaceNormal(std::span<vec3 const> face) noexcept
    {
        return ngonFaceNormalScaled(face).normalized();
    }

    /// <summary>
    /// Calculates the scaled normal for a set of vertices defining an n-gon in counter-clockwise order.
    /// The scaled, unnormalized, value is typically used for area weighting, degeneracy tests, etc.
    /// </summary>
    [[nodiscard]] constexpr vec3 ngonFaceNormalScaledCCW(std::span<Vertex const> face) noexcept
    {
        // Using Newell's method
        vec3 normal{};

        if (face.size() >= 3)
        {
            for (size_t i = 0, j = face.size() - 1; i < face.size(); j = i++)       // note that j starts at the end, and then follows one behind
            {
                const vec3& curr = face[j].position;
                const vec3& next = face[i].position;

                normal.x() += (curr.y() - next.y()) * (curr.z() + next.z());        // note this looks like the cross product, but its not.
                normal.y() += (curr.z() - next.z()) * (curr.x() + next.x());
                normal.z() += (curr.x() - next.x()) * (curr.y() + next.y());
            }
        }

        return normal;
    }

    /// <summary>
    /// Calculates the normal for a set of vertices defining an n-gon in counter-clockwise order.
    /// </summary>
    [[nodiscard]] constexpr vec3 ngonFaceNormalCCW(std::span<Vertex const> face) noexcept
    {
        return ngonFaceNormalScaledCCW(face).normalized();
    }

    /// <summary>
    /// Calculates the scaled normal for a set of vertices defining an n-gon in clockwise order.
    /// The scaled, unnormalized, value is typically used for area weighting, degeneracy tests, etc.
    /// </summary>
    [[nodiscard]] constexpr vec3 ngonFaceNormalScaled(std::span<Vertex const> face) noexcept
    {
        // Negate the accumulated normal to flip the winding to clockwise
        return -ngonFaceNormalScaledCCW(face);
    }

    /// <summary>
    /// Calculates the normal for a set of vertices defining an n-gon in counter-clockwise order.
    /// </summary>
    [[nodiscard]] constexpr vec3 ngonFaceNormal(std::span<Vertex const> face) noexcept
    {
        return ngonFaceNormalScaled(face).normalized();
    }

    /// <summary>
    /// Given a mesh defined by its verticies and indices, recalculates the normals for each vertex.
    /// This function expects the mesh to be fully triangulated (all faces composed of 3 points) and 
    /// for all face indices to be ordered in clockwise winding.
    /// </summary>
    void calculateMeshNormals(std::span<Vertex> vertices, std::span<uint32_t> indices) noexcept;
}

#endif