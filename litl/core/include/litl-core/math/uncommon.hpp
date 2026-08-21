#ifndef LITL_MATH_UNCOMMON_H__
#define LITL_MATH_UNCOMMON_H__

#include <cstdint>
#include <span>

#include "litl-core/math/common.hpp"
#include "litl-core/math/types.hpp"
#include "litl-core/math/geometry/vertex.hpp"

namespace litl
{
    /// <summary>
    /// Given the three positions comprising a face (in counter-clockwise order), returns the scaled face normal.
    /// The scaled, unnormalized, value is typically used for area weighting, degeneracy tests, etc.
    /// </summary>
    [[nodiscard]] constexpr vec3 faceNormalScaledCCW(vec3 a, vec3 b, vec3 c) noexcept
    {
        return cross(b - a, c - a);    // face area = (length / 2); area near 0 = degenerate triangle
    }

    /// <summary>
    /// Given the three positions comprising a face (in counter-clockwise order), returns the face normal.
    /// </summary>
    [[nodiscard]] constexpr vec3 faceNormalCCW(vec3 a, vec3 b, vec3 c) noexcept
    {
        vec3 scaledNormal = faceNormalScaledCCW(a, b, c);

        if (scaledNormal.lengthSquared() > Traits<float>::epsilon)
        {
            scaledNormal.normalize();
        }

        return scaledNormal;
    }

    /// <summary>
    /// Given the three positions comprising a face (in clockwise order), returns the scaled face normal.
    /// The scaled, unnormalized, value is typically used for area weighting, degeneracy tests, etc.
    /// 
    /// Internally, LITL defaults to clockwise winding due to its left-handed coordinate system.
    /// 
    /// The magnitude of the resulting normal vector is equals to 2 * area of the polygon.
    /// </summary>
    [[nodiscard]] constexpr vec3 faceNormalScaled(vec3 a, vec3 b, vec3 c) noexcept
    {
        return cross(c - a, b - a);    // face area = (length / 2); area near 0 = degenerate triangle
    }

    /// <summary>
    /// Given the three positions comprising a face (in clockwise order), returns the face normal.
    /// Internally, LITL defaults to clockwise winding due to its left-handed coordinate system.
    /// </summary>
    [[nodiscard]] constexpr vec3 faceNormal(vec3 a, vec3 b, vec3 c) noexcept
    {
        vec3 scaledNormal = faceNormalScaled(a, b, c);

        if (scaledNormal.lengthSquared() > Traits<float>::epsilon)
        {
            scaledNormal.normalize();
        }

        return scaledNormal;
    }

    /// <summary>
    /// Calculates the scaled normal for a set of vertex positions defining an n-gon in counter-clockwise order.
    /// The scaled, unnormalized, value is typically used for area weighting, degeneracy tests, etc.
    /// 
    /// The magnitude of the resulting normal vector is equals to 2 * area of the polygon.
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
        vec3 scaledNormal = ngonFaceNormalScaledCCW(face);

        if (scaledNormal.lengthSquared() > Traits<float>::epsilon)
        {
            scaledNormal.normalize();
        }

        return scaledNormal;
    }

    /// <summary>
    /// Calculates the scaled normal for a set of vertex positions defining an n-gon in clockwise order.
    /// The scaled, unnormalized, value is typically used for area weighting, degeneracy tests, etc.
    /// 
    /// The magnitude of the resulting normal vector is equals to 2 * area of the polygon.
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
        return -ngonFaceNormalCCW(face);
    }

    /// <summary>
    /// Calculates the scaled normal for a set of vertices defining an n-gon in counter-clockwise order.
    /// The scaled, unnormalized, value is typically used for area weighting, degeneracy tests, etc.
    /// 
    /// The magnitude of the resulting normal vector is equals to 2 * area of the polygon.
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
        vec3 scaledNormal = ngonFaceNormalScaledCCW(face);

        if (scaledNormal.lengthSquared() > Traits<float>::epsilon)
        {
            scaledNormal.normalize();
        }

        return scaledNormal;
    }

    /// <summary>
    /// Calculates the scaled normal for a set of vertices defining an n-gon in clockwise order.
    /// The scaled, unnormalized, value is typically used for area weighting, degeneracy tests, etc.
    /// 
    /// The magnitude of the resulting normal vector is equals to 2 * area of the polygon.
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
        return -ngonFaceNormalCCW(face);
    }

    /// <summary>
    /// Duff et al., "Building an Orthonormal Basis, Revisited" (JCGT 2017).
    /// Branchless, no precision cliff at n.z == -1.
    /// 
    /// Given a unit normal, produces unit vectors tangent and bitangent spanning the plane orthogonal to the normal, such that: bitangent = cross(normal, tangent). 
    /// Equivalently, negate the bitangent if you need to reverse winding such that: normal == cross(tangent, bitangent). For example if a left-handed orthonormal basis is needed.
    /// 
    /// An example of an orthonormal basis in 3D is the classic:
    /// 
    ///     right = (1, 0, 0)
    ///        up = (0, 1, 0)
    ///   forward = (0, 0, 1)
    /// </summary>
    constexpr void orthonormalBasisRH(vec3 normal, vec3& tangent, vec3& bitangent) noexcept
    {
        const float sign = std::copysignf(1.0f, normal.z());
        const float a = -1.0f / (sign + normal.z());
        const float c = normal.x() * normal.y() * a;

        tangent = vec3 {
            (1.0f + (normal.x() * normal.x() * a * sign)),
            (c * sign),
            (normal.x() * -sign)};

        bitangent = vec3{
            c,
            sign + (normal.y() * normal.y() * a),
            -normal.y()};
    }

    /// <summary>
    /// Variation of orthonormalBasis which produces a left-handed orthonormal basis.
    /// </summary>
    constexpr void orthonormalBasis(vec3 normal, vec3& tangent, vec3& bitangent) noexcept
    {
        orthonormalBasisRH(normal, tangent, bitangent);
        bitangent = -bitangent;
    }

    /// <summary>
    /// Projects the 3D point onto a 2D plane defined by its normal.
    /// The provided planeNormal must be a unit vector.
    /// </summary>
    [[nodiscard]] constexpr vec2 project2d(vec3 planeNormal, vec3 point, vec3 origin) noexcept
    {
        if (point.isZeroed())
        {
            return vec2::zero();
        }

        const vec3 po = point - origin;
        vec3 t, b; orthonormalBasis(planeNormal, t, b);

        return vec2{ dot(po, t), dot(po, b) };
    }

    /// <summary>
    /// Returns the signed area of a 2D polygon/face using the shoelace method.
    /// </summary>
    [[nodiscard]] constexpr float polygonSignedArea(std::span<vec2 const> face) noexcept
    {
        const size_t n = face.size();
        float sum = 0.0f;

        for (size_t i = 0ull; i < n; ++i)
        {
            const auto& a = face[i];
            const auto& b = face[(i + 1) % n];
            sum += (a.x() * b.y()) - (b.x() * a.y());
        }

        return (sum * 0.5f);
    }

    /// <summary>
    /// Returns the unsigned area of a 2D polygon/face using the shoelace method.
    /// </summary>
    [[nodiscard]] constexpr float polygonArea(std::span<vec2 const> face) noexcept
    {
        return abs(polygonSignedArea(face));
    }

    /// <summary>
    /// Returns the unsigned area of a 3D polygon/face using the newell method.
    /// </summary>
    [[nodiscard]] constexpr float polygonArea(std::span<vec3 const> face) noexcept
    {
        return ngonFaceNormalScaledCCW(face).length() * 0.5f;  // |newell| == area * 2
    }

    /// <summary>
    /// Returns true if the provided polygon/face is specified in clockwise winding order.
    /// </summary>
    [[nodiscard]] constexpr bool isFaceCW(std::span<vec2 const> face) noexcept
    {
        return (polygonSignedArea(face) < 0.0f);
    }

    /// <summary>
    /// Returns true if the provided polygon/face is specified in counter-clockwise winding order.
    /// </summary>
    [[nodiscard]] constexpr bool isFaceCCW(std::span<vec2 const> face) noexcept
    {
        return (polygonSignedArea(face) >= 0.0f);
    }

    /// <summary>
    /// Returns true if the provided polygon/face is specified in clockwise winding order.
    /// 
    /// For this 3D form, a reference normal is needed to compare against. This could be
    /// the vector from a known interior point (centroid, bounds center, etc.) to the face's
    /// centroid. If they point the same way then the face is oriented outwards.
    /// </summary>
    [[nodiscard]] constexpr bool isFaceCW(std::span<vec3 const> face, vec3 referenceNormal) noexcept
    {
        const vec3 faceNormal = ngonFaceNormal(face);
        return dot(faceNormal, referenceNormal) >= 0.0f;
    }

    /// <summary>
    /// Returns true if the provided polygon/face is specified in counter-clockwise winding order.
    /// 
    /// For this 3D form, a reference normal is needed to compare against. This could be
    /// the vector from a known interior point (centroid, bounds center, etc.) to the face's
    /// centroid. If they point the same way then the face is oriented outwards.
    /// </summary>
    [[nodiscard]] constexpr bool isFaceCCW(std::span<vec3 const> face, vec3 referenceNormal) noexcept
    {
        const vec3 faceNormal = ngonFaceNormalCCW(face);
        return dot(faceNormal, referenceNormal) >= 0.0f;
    }
}

#endif