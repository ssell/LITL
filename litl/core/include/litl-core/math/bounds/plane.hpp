#ifndef LITL_MATH_BOUNDS_PLANE_H__
#define LITL_MATH_BOUNDS_PLANE_H__

#include "litl-core/math/bounds/intersectionResult.hpp"
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
            return Plane{ normal, dot(normal, point) };
        }

        /// <summary>
        /// Calculates a plane from the three points of a triangle.
        /// This is the default winding used - as opposed to the renderer which uses clockwise due to the y-flip.
        /// </summary>
        /// <param name="a"></param>
        /// <param name="b"></param>
        /// <param name="c"></param>
        /// <returns></returns>
        [[nodiscard]] static constexpr Plane fromTriangleCCW(vec3 a, vec3 b, vec3 c) noexcept
        {
            // cross product "encodes" a finite patch of a plane. normalizing the result gives the normal of the plane.
            // n · p - d = 0
            auto n = cross(b - a, c - a).normalized();
            auto d = dot(n, a);
            return Plane{ n, d };
        }

        /// <summary>
        /// Calculates a plane from the three points of a triangle.
        /// </summary>
        /// <param name="a"></param>
        /// <param name="b"></param>
        /// <param name="c"></param>
        /// <returns></returns>
        [[nodiscard]] static constexpr Plane fromTriangleCW(vec3 a, vec3 b, vec3 c) noexcept
        {
            return fromTriangleCCW(a, c, b);
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

    // -------------------------------------------------------------------------------------
    // Plane x Plane Intersection
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Calculates the intersection point of three planes.
    /// </summary>
    /// <param name="a"></param>
    /// <param name="b"></param>
    /// <param name="c"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr IntersectionResult intersects(Plane a, Plane b, Plane c)
    {
        IntersectionResult result{ .intersects = false };

        vec3 crossBC = cross(b.normal(), c.normal());
        float determinant = dot(a.normal(), crossBC);

        // If the determinant is near zero, the planes are parallel or intersect at a line.
        // Otherwise the three planes intersect at a single point.
        if (!isZero(determinant))
        {
            vec3 crossAB = cross(a.normal(), b.normal());
            vec3 crossCA = cross(c.normal(), a.normal());

            result.intersects = true;
            result.point = ((crossBC * a.d()) + (crossCA * b.d()) + (crossAB * c.d())) / determinant;
        }

        return result;
    }
}

#endif