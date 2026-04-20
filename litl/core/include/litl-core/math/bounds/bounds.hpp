#ifndef LITL_MATH_BOUNDS_H__
#define LITL_MATH_BOUNDS_H__

#include "litl-core/math/bounds/intersectionResult.hpp"
#include "litl-core/math/bounds/aabb.hpp"
#include "litl-core/math/bounds/sphere.hpp"
#include "litl-core/math/bounds/plane.hpp"
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

    // -------------------------------------------------------------------------------------
    // Plane
    // -------------------------------------------------------------------------------------

    // -------------------------------------------------------------------------------------
    // Plane x Point

    /// <summary>
    /// An inclusive contains check to see if a point lies on, or within, a plane.
    /// A point is considered inside a plane if its signed distance is >= 0.
    /// </summary>
    /// <param name="plane"></param>
    /// <param name="point"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool contains(Plane plane, vec3 point) noexcept
    {
        return plane.signedDistance(point) >= 0.0f;
    }

    // -------------------------------------------------------------------------------------
    // Plane x Sphere

    /// <summary>
    /// An inclusive contains check to see if a sphere is completely within the plane.
    /// </summary>
    /// <param name="plane"></param>
    /// <param name="sphere"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool contains(Plane plane, Sphere sphere) noexcept
    {
        return plane.signedDistance(sphere.center) >= sphere.radius;
    }

    /// <summary>
    /// Calculates it the sphere straddles the plane - neither fully within or outside.
    /// </summary>
    /// <param name="plane"></param>
    /// <param name="sphere"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool intersects(Plane plane, Sphere sphere) noexcept
    {
        return abs(plane.signedDistance(sphere.center)) < sphere.radius;
    }

    /// <summary>
    /// Calculates it the sphere lies completely outside the plane.
    /// </summary>
    /// <param name="plane"></param>
    /// <param name="sphere"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool isOutside(Plane plane, Sphere sphere) noexcept
    {
        return plane.signedDistance(sphere.center) < -sphere.radius;
    }

    // -------------------------------------------------------------------------------------
    // Plane x AABB

    /// <summary>
    /// An inclusive contains check to see if an AABB is completely within the plane.
    /// </summary>
    /// <param name="plane"></param>
    /// <param name="aabb"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool contains(Plane plane, AABB aabb) noexcept
    {
        return false;
    }

    /// <summary>
    /// Calculates it the AABB straddles the plane - neither fully within or outside.
    /// </summary>
    /// <param name="plane"></param>
    /// <param name="aabb"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool intersects(Plane plane, AABB aabb) noexcept
    {
        return false;
    }

    /// <summary>
    /// Calculates it the AABB lies completely outside the plane.
    /// </summary>
    /// <param name="plane"></param>
    /// <param name="aabb"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool isOutside(Plane plane, AABB aabb) noexcept
    {
        return false;
    }

    // -------------------------------------------------------------------------------------
    // Frustum
    // -------------------------------------------------------------------------------------

    // -------------------------------------------------------------------------------------
    // Frustum x Point

    /// <summary>
    /// Calculates the rejection bitmask (outcode) for the given point and frustum.
    /// Each set bit corresponds to a plane that the point is outside of.
    /// </summary>
    /// <param name="frustum"></param>
    /// <param name="point"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr uint32_t containsMask(Frustum const& frustum, vec3 point) noexcept
    {
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

        uint32_t rejectionMask = 0;

        for (uint32_t i = 0u; i < frustum.sideCount(); ++i)
        {
            rejectionMask |= static_cast<uint32_t>(!contains(frustum.getSide(static_cast<Frustum::Side>(i)), point)) << i;
        }

        return rejectionMask;
    }

    /// <summary>
    /// An inclusive contains check between a frustum and a single point.
    /// A point is contained within a frustum if it is also contained within all of its planes.
    /// </summary>
    /// <param name="frustum"></param>
    /// <param name="point"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool contains(Frustum const& frustum, vec3 point) noexcept
    {
        return containsMask(frustum, point) == 0;
    }

    // -------------------------------------------------------------------------------------
    // Frustum x Sphere

    /// <summary>
    /// An inclusive contains check between a frustum and a sphere.
    /// A sphere is contained within a frustum if it is also contained within all of its planes.
    /// </summary>
    /// <param name="frustum"></param>
    /// <param name="sphere"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool contains(Frustum const& frustum, Sphere sphere) noexcept
    {
        bool allContains = true;

        for (uint32_t i = 0u; i < frustum.sideCount(); ++i)
        {
            allContains = allContains && contains(frustum.getSide(static_cast<Frustum::Side>(i)), sphere);
        }

        return allContains;
    }

    /// <summary>
    /// A sphere intersects with a frustum if it is not fully outside any single plane.
    /// </summary>
    /// <param name="frustum"></param>
    /// <param name="sphere"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool intersects(Frustum const& frustum, Sphere sphere) noexcept
    {
        bool allIntersects = true;

        for (uint32_t i = 0u; i < frustum.sideCount(); ++i)
        {
            allIntersects = allIntersects && !isOutside(frustum.getSide(static_cast<Frustum::Side>(i)), sphere);
        }

        return allIntersects;
    }

    // -------------------------------------------------------------------------------------
    // Frustum Intersection Classification

    struct FrustumClassification
    {
        enum Type
        {
            /// <summary>
            /// The object is fully contained within all frustum planes.
            /// </summary>
            Inside = 0,

            /// <summary>
            /// The object is not fully outside any plane, but intersects at least one frustum plane.
            /// </summary>
            Intersects,

            /// <summary>
            /// The object is fully outside at least one frustum plane.
            /// </summary>
            Outside
        };

        /// <summary>
        /// Each bit correlates to a frustum plane that the object is fully outside of.
        /// </summary>
        uint32_t outsideMask{ 0 };

        /// <summary>
        /// Each bit correlates to a frustum plane that the object straddles/intersects.
        /// </summary>
        uint32_t straddleMask{ 0 };

        [[nodiscard]] constexpr Type type() const noexcept
        {
            if (outsideMask != 0)
            {
                return Outside;
            }
            else if (straddleMask != 0)
            {
                return Intersects;
            }
            else
            {
                return Inside;
            }
        }
    };

    /// <summary>
    /// Classifies how the sphere intersects (or doesn't) with the frustum.<br/>
    /// 
    /// The active mask, if specified, is the parent's (straddleMask | outsideMask) which is used
    /// to skip planes that are we already know completely contain the parent (and thus) the child.
    /// </summary>
    /// <param name="frustum"></param>
    /// <param name="sphere"></param>
    /// <param name="activeMask"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr FrustumClassification classify(Frustum const& frustum, Sphere sphere, uint32_t activeMask = 0b111111) noexcept
    {
        FrustumClassification result{};

        for (uint32_t i = 0u; i < frustum.sideCount(); ++i)
        {
            const uint32_t maskBit = 1u << i;

            if ((activeMask & maskBit) == 0u)
            {
                // the plane fully contains the parent, and thus this child (straddleBit | outsideBit = 0)
                continue;
            }

            const float dist = frustum.getSide(static_cast<Frustum::Side>(i)).signedDistance(sphere.center);

            if (dist < -sphere.radius)
            {
                // once the sphere is fully outside a single plane then it is classified as outside.
                result.outsideMask |= maskBit;
                return result;
            }
            
            if (dist <= sphere.radius)
            {
                result.straddleMask |= maskBit;
            }
        }

        return result;
    }
}

#endif