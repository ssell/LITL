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

    /// <summary>
    /// 
    /// </summary>
    /// <param name="frustum"></param>
    /// <param name="point"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool contains(Frustum const& frustum, vec3 point) noexcept
    {
        uint32_t rejectionMask = 0;

        /**
         * Iterate all sides, setting their corresponding bit mask value to 1 if they were rejected.
         * 
         * Example,
         * 
         *     0b0000'0000 = no rejections (pass)
         *     0b0011'1111 (or 0b0001'1111 if infinite far) = all rejected
         *     0b0000'0011 = left and right rejected
         * 
         * See Frustum::Sides
         */

        for (uint32_t i = 0u; i < frustum.sideCount(); ++i)
        {
            rejectionMask |= static_cast<uint32_t>(!contains(frustum.getSide(i), point)) << i;
        }

        // In the future, this rejection mask is used by Cohen-Sutherland and Sutherland-Hodgman clipping
        // and can be used in hierarchical culling (if all points of a child share a rejection bit, then
        // the entire child is outside the plane).

        return (rejectionMask == 0);
    }
}

#endif