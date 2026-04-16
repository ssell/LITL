#ifndef LITL_MATH_BOUNDS_H__
#define LITL_MATH_BOUNDS_H__

#include "litl-core/math/bounds/aabb.hpp"
#include "litl-core/math/bounds/sphere.hpp"
#include "litl-core/math/bounds/plane.hpp"
#include "litl-core/math/bounds/frustum.hpp"

namespace litl::bounds
{
    // -------------------------------------------------------------------------------------
    // Contains
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// An inclusive contains check between an axis-aligned bounding box and a point.
    /// Returns true if the point is on or within the bounds.
    /// </summary>
    /// <param name="bounds"></param>
    /// <param name="point"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool contains(AABB bounds, vec3 point) noexcept
    {
        return
            (point.x() >= bounds.min.x()) && (point.x() <= bounds.max.x()) &&
            (point.y() >= bounds.min.y()) && (point.y() <= bounds.max.y()) &&
            (point.z() >= bounds.min.z()) && (point.z() <= bounds.max.z());
    }

    /// <summary>
    /// An inclusive contains check between an axis-aligned bounding box and a point.
    /// Returns true if the point is on or within the bounds.
    /// </summary>
    /// <param name="bounds"></param>
    /// <param name="point"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool contains(Sphere bounds, vec3 point) noexcept
    {
        return (bounds.center.distanceSqTo(point) <= (bounds.radius * bounds.radius));
    }

    /// <summary>
    /// An inclusive contains check to see if a point lies on, or within, a plane.
    /// </summary>
    /// <param name="plane"></param>
    /// <param name="point"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool contains(Plane plane, vec3 point) noexcept
    {
        return plane.signedDistance(point) <= Traits<float>::relative_epsilon;
    }
}

#endif