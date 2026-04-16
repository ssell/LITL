#ifndef LITL_MATH_BOUNDS_PLANE_H__
#define LITL_MATH_BOUNDS_PLANE_H__

#include "litl-core/math/traits.hpp"
#include "litl-core/math/types/vec3.hpp"
#include "litl-core/math/types/vec4.hpp"

namespace litl::bounds
{
    /// <summary>
    /// Plane stored in Hessian normal form:
    /// 
    ///     dot(n, p) = d
    /// 
    /// Where:
    /// 
    ///     n = normal
    ///     d = signed distance from origin
    ///     p = point on the plane
    /// 
    /// Stored as:
    /// 
    ///     xyz = n
    ///     w = d
    /// </summary>
    struct Plane
    {
        constexpr Plane() {}
        constexpr Plane(float nx, float ny, float nz, float d) : value{ nx, ny, nz, d } {};
        constexpr Plane(vec3 normal, float d) : value{ normal.x(), normal.y(), normal.z(), d } {};
        constexpr explicit Plane(vec4 plane) : value{ plane } {};

        vec4 value{ 0.0f, 1.0f, 0.0f, 0.0f };

        [[nodiscard]] static constexpr Plane fromPointNormal(vec3 point, vec3 normal) noexcept
        {
            return Plane{ normal, normal.dot(point) };
        }

        [[nodiscard]] static constexpr Plane fromTriangle(vec3 a, vec3 b, vec3 c) noexcept
        {
            // cross product "encodes" a finite patch of a plane.
            // normalizing the result gives the normal of the plane.
            return fromPointNormal(a, (b - a).cross(c - a).normalized());
        }

        [[nodiscard]] constexpr vec3 normal() const noexcept
        {
            return vec3{ value.x(), value.y(), value.z() };
        }

        /// <summary>
        /// Not all planes need to be normalized. <br/>
        /// 
        /// You need normalization if you care about true Euclidean signed distance 
        /// such as for LOD selection based on distance-from-plane, for conservative shadow fitting, 
        /// for any query where "how far from the plane" is the answer. <br/>
        /// 
        /// You can skip normalization if you only care about sign (inside/outside tests), since 
        /// multiplying the plane by a positive scalar doesn't change which side a point is on.
        /// </summary>
        void normalize() noexcept
        {
            const float lengthInv = 1.0f / sqrtf((value.x() * value.x()) + (value.y() * value.y()) + (value.z() * value.z()));
            value *= lengthInv;
        }

        [[nodiscard]] constexpr float d() const noexcept
        {
            return value.w();
        }

        [[nodiscard]] constexpr float signedDistance(vec3 point) const noexcept
        {
            // the "center" of the plane lies on the normal projected out from the origin.
            // by doing (normal • point) we project the point onto that normal and get it's distance 
            // from the origin. from there simply subtract the distance the point lies from the origin 
            return (normal().dot(point) - d());
        }
    };
}

#endif