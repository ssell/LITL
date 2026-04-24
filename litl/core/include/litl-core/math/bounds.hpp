#ifndef LITL_MATH_BOUNDS_H__
#define LITL_MATH_BOUNDS_H__

#include "litl-core/math/bounds/intersectionResult.hpp"
#include "litl-core/math/bounds/aabb.hpp"
#include "litl-core/math/bounds/sphere.hpp"
#include "litl-core/math/bounds/plane.hpp"
#include "litl-core/math/bounds/frustum.hpp"

namespace litl::bounds
{
    enum class IntersectionType
    {
        /// <summary>
        /// The second object is fully within the first.
        /// </summary>
        Inside = 0,

        /// <summary>
        /// The objects intersect each other (neither fully inside or outside).
        /// </summary>
        Intersects,

        /// <summary>
        /// The second object is fully outside the first.
        /// </summary>
        Outside
    };

    struct FrustumClassification
    {
        /// <summary>
        /// Each bit correlates to a frustum plane that the object is fully outside of.
        /// </summary>
        uint32_t outsideMask{ 0 };

        /// <summary>
        /// Each bit correlates to a frustum plane that the object straddles/intersects.
        /// </summary>
        uint32_t straddleMask{ 0 };

        [[nodiscard]] constexpr IntersectionType type() const noexcept
        {
            if (outsideMask != 0)
            {
                return IntersectionType::Outside;
            }
            else if (straddleMask != 0)
            {
                return IntersectionType::Intersects;
            }
            else
            {
                return IntersectionType::Inside;
            }
        }
    };

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
        // If the negative vertex (the one that lies farthest along the negative normal) is within the plane then the entire AABB is within the plane.
        if (plane.signedDistance(aabb.nVertex(plane.normal())) >= 0.0f)
        {
            return true;
        }

        // Otherwise, part or all of the AABB lies outside the plane.
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
        const vec3 normal = plane.normal();

        // If the negative vertex (the one that lies farthest along the negative normal) is within the plane then the entire AABB is within the plane.
        if (plane.signedDistance(aabb.nVertex(normal)) >= 0.0f)
        {
            return false;
        }

        // If the positive vertex (the one that lies farthest along the normal) is outside the plane then the entire AABB is outside the plane.
        if (plane.signedDistance(aabb.pVertex(normal)) < 0.0f)
        {
            return false;
        }

        // The AABB straddles the plane.
        return true;
    }

    /// <summary>
    /// Calculates it the AABB lies completely outside the plane.
    /// </summary>
    /// <param name="plane"></param>
    /// <param name="aabb"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool isOutside(Plane plane, AABB aabb) noexcept
    {
        // If the positive vertex (the one that lies farthest along the normal) is outside the plane then the entire AABB is outside the plane.
        if (plane.signedDistance(aabb.pVertex(plane.normal())) < 0.0f)
        {
            return true;
        }

        // Otherwise, part or all of the AABB lies inside the plane.
        return false;
    }

    // -------------------------------------------------------------------------------------
    // AABB
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
    /// Calculates if the first AABB fully contains the second.
    /// </summary>
    /// <param name="aabb"></param>
    /// <param name="other"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool contains(AABB aabb, AABB other) noexcept
    {
        return contains(aabb, other.min) && contains(aabb, other.max);
    }

    /// <summary>
    /// Calculates if the two AABBs are intersecting (one or both min/max points within).
    /// </summary>
    /// <param name="aabb"></param>
    /// <param name="other"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool intersects(AABB a, AABB b) noexcept
    {
        return
            (a.min.x() <= b.max.x() && a.max.x() >= b.min.x()) &&
            (a.min.y() <= b.max.y() && a.max.y() >= b.min.y()) &&
            (a.min.z() <= b.max.z() && a.max.z() >= b.min.z());

    }

    /// <summary>
    /// Returns true if the sphere fully contains or partially intersects the AABB.
    /// </summary>
    /// <param name="sphere"></param>
    /// <param name="aabb"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool intersects(Sphere sphere, AABB aabb) noexcept
    {
        return aabb.distanceSqTo(sphere.center) <= (sphere.radius * sphere.radius);
    }

    /// <summary>
    /// Classifies if the second AABB is inside, outside, or intersects the first.
    /// </summary>
    /// <param name="aabb"></param>
    /// <param name="other"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr IntersectionType classify(AABB aabb, AABB other) noexcept
    {
        const bool minInside = contains(aabb, other.min);
        const bool maxInside = contains(aabb, other.max);

        if (minInside && maxInside)
        {
            return IntersectionType::Inside;
        }
        else
        {
            if (intersects(aabb, other))
            {
                return IntersectionType::Intersects;
            }
            else
            {
                return IntersectionType::Outside;
            }
        }
    }

    /// <summary>
    /// Is the sphere fully contained within the AABB?
    /// </summary>
    /// <param name="aabb"></param>
    /// <param name="sphere"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool contains(AABB aabb, Sphere sphere) noexcept
    {
        return 
            (sphere.center.x() - sphere.radius) >= aabb.min.x() &&
            (sphere.center.x() + sphere.radius) <= aabb.max.x() &&
            (sphere.center.y() - sphere.radius) >= aabb.min.y() &&
            (sphere.center.y() + sphere.radius) <= aabb.max.y() &&
            (sphere.center.z() - sphere.radius) >= aabb.min.z() &&
            (sphere.center.z() + sphere.radius) <= aabb.max.z();
    }

    /// <summary>
    /// Returns true if the sphere is within or partially intersects the AABB. Otherwise, returns false.
    /// </summary>
    /// <param name="aabb"></param>
    /// <param name="sphere"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr bool intersects(AABB aabb, Sphere sphere) noexcept
    {
        const float distSq = aabb.distanceSqTo(sphere.center);      // 0 if the sphere center is within the AABB
        const float radSq = sphere.radius * sphere.radius;

        return distSq <= radSq;
    }

    /// <summary>
    /// Classifies if the Sphere is inside, outside, or intersects the AABB.
    /// </summary>
    /// <param name="aabb"></param>
    /// <param name="other"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr IntersectionType classify(AABB aabb, Sphere sphere) noexcept
    {
        if (contains(aabb, sphere))
        {
            return IntersectionType::Inside;
        }

        return intersects(aabb, sphere) ? IntersectionType::Intersects : IntersectionType::Outside;
    }

    /// <summary>
    /// Returns if the aabb is fully within, outside, or intersects the sphere.
    /// </summary>
    /// <param name="sphere"></param>
    /// <param name="aabb"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr IntersectionType classify(Sphere sphere, AABB aabb) noexcept
    {
        const float radiusSq = sphere.radius * sphere.radius;
        const float closestDistSq = aabb.distanceSqTo(sphere.center);

        // If the closest point is outside the sphere, then the entire AABB is outside.
        if (closestDistSq > radiusSq)
        {
            return IntersectionType::Outside;
        }

        // Find the farthest point on the AABB from the sphere center;
        const vec3 d1 = abs(sphere.center - aabb.min);
        const vec3 d2 = abs(sphere.center - aabb.max);
        const vec3 farthest = max(d1, d2);
        const float farthestDistSq = dot(farthest, farthest);

        // If the farthest corner is inside the sphere, then the entire ABB is inside.
        if (farthestDistSq <= radiusSq)
        {
            return IntersectionType::Inside;
        }

        return IntersectionType::Intersects;
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
                // the plane fully contains the parent, and thus contains this child as well (straddleBit | outsideBit = 0)
                continue;
            }

            const float dist = frustum.getSide(static_cast<Frustum::Side>(i)).signedDistance(sphere.center);

            // If the sphere is fully outside, then well it is outside and mark the mask as such.
            if (dist < -sphere.radius)
            {
                result.outsideMask |= maskBit;
                return result;
            }
            
            // If the sphere is partially inside, then it straddles.
            if (dist <= sphere.radius)
            {
                result.straddleMask |= maskBit;
            }

            // Otherwise the sphere is fully inside - so do nothing/set no bits.
        }

        return result;
    }

    // -------------------------------------------------------------------------------------
    // Frustum x AABB

    [[nodiscard]] constexpr bool contains(Frustum const& frustum, AABB aabb) noexcept
    {
        for (uint32_t i = 0u; i < frustum.sideCount(); ++i)
        {
            const auto& plane = frustum.getSide(static_cast<Frustum::Side>(i));

            if (!contains(plane, aabb))
            {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] constexpr FrustumClassification classify(Frustum const& frustum, AABB aabb, uint32_t activeMask = 0b111111) noexcept
    {
        FrustumClassification result{};

        for (uint32_t i = 0u; i < frustum.sideCount(); ++i)
        {
            const uint32_t maskBit = 1u << i;

            if ((activeMask & maskBit) == 0u)
            {
                // the plane fully contains the parent, and thus contains this child as well (straddleBit | outsideBit = 0)
                continue;
            }

            const auto& plane = frustum.getSide(static_cast<Frustum::Side>(i));
            const vec3 normal = plane.normal();
            const vec3 pVert = aabb.pVertex(normal);

            // If the p-vertex is outside the plane, then the entire AABB is outside.
            if (plane.signedDistance(pVert) < 0.0f)
            {
                result.outsideMask |= maskBit;
                return result;
            }

            const vec3 nVert = aabb.nVertex(normal);

            // If the n-vertex is outside the plane (and the above p-vert we know is inside), then the AABB straddles
            if (plane.signedDistance(nVert) < 0.0f)
            {
                result.straddleMask |= maskBit;
            }

            // Otherwise, the aabb is fully within the plane - so do nothing/set no bits.
        }

        return result;
    }

    [[nodiscard]] constexpr bool intersects(Frustum const& frustum, AABB aabb) noexcept
    {
        return classify(frustum, aabb).type() != IntersectionType::Outside;
    }

    // -------------------------------------------------------------------------------------
    // Compute
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Returns a Spherw which encloses the provided AABB.
    /// </summary>
    /// <param name="aabb"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr Sphere computeSphere(AABB const& aabb) noexcept
    {
        return Sphere::fromCenterRadius(aabb.center(), aabb.halfExtents().length());
    }

    /// <summary>
    /// Returns an AABB which encloses the provided Frustum.
    /// </summary>
    /// <param name="frustum"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr AABB computeAABB(Frustum const& frustum) noexcept
    {
        auto corners = Frustum::extractCorners(frustum);
        return AABB::fromMinMax(corners.calculateMinPoint(), corners.calculateMaxPoint());
        
    }
}

#endif