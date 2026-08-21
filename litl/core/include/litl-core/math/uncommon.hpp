#ifndef LITL_MATH_UNCOMMON_H__
#define LITL_MATH_UNCOMMON_H__

#include "litl-core/math/common.hpp"
#include "litl-core/math/types.hpp"

namespace litl
{
    /// <summary>
    /// Projects the 3D point onto a 2D plane defined by its normal.
    /// </summary>
    [[nodiscard]] constexpr vec2 project2d(vec3 planeNormal, vec3 point, vec3 origin) noexcept
    {
        if (point.isZeroed())
        {
            return vec2::zero();
        }

        const vec3 po = point - origin;
        const vec3 t = tangent(planeNormal);
        const vec3 b = cross(planeNormal, t);

        return vec2{ dot(po, t), dot(po, b) };
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
}

#endif