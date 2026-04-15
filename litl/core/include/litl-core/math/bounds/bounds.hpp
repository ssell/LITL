#ifndef LITL_MATH_BOUNDS_H__
#define LITL_MATH_BOUNDS_H__

#include "litl-core/math/bounds/aabb.hpp"
#include "litl-core/math/bounds/sphere.hpp"
#include "litl-core/math/bounds/frustum.hpp"

namespace litl::bounds
{
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
}

#endif