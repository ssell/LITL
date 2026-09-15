#ifndef LITL_MATH_BOUNDS_AABB_H__
#define LITL_MATH_BOUNDS_AABB_H__

#include <array>
#include <span>
#include <type_traits>

#include "litl-core/assert.hpp"
#include "litl-core/math/types/vec3.hpp"
#include "litl-core/math/types/mat4.hpp"

namespace litl::bounds
{
    /// <summary>
    /// A basic axis-aligned bounding box.
    /// No check is performed as to the "correctness" of the min and max values.
    /// </summary>
    struct AABB
    {
        vec3 min{ 0.0f, 0.0f, 0.0f };
        vec3 max{ 0.0f, 0.0f, 0.0f };

        [[nodiscard]] bool operator==(AABB const& other) const noexcept
        {
            return (min == other.min) && (max == other.max);
        }

        /// <summary>
        /// Returns the distance between the min/max points.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] constexpr vec3 extents() const noexcept
        {
            return (max - min);
        }

        /// <summary>
        /// Returns the distance between the center point and the min/max points.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] constexpr vec3 halfExtents() const noexcept
        {
            return (max - min) * 0.5f;
        }

        /// <summary>
        /// Returns the center point between the min/max points.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] constexpr vec3 center() const noexcept
        {
            return (min + ((max - min) * 0.5f));
        }

        [[nodiscard]] constexpr std::array<vec3, 8> corners() const noexcept
        {
            return {{
                vec3{ min.x(), min.y(), min.z() },
                vec3{ max.x(), min.y(), min.z() },
                vec3{ min.x(), max.y(), min.z() },
                vec3{ max.x(), max.y(), min.z() },
                vec3{ min.x(), min.y(), max.z() },
                vec3{ max.x(), min.y(), max.z() },
                vec3{ min.x(), max.y(), max.z() },
                vec3{ max.x(), max.y(), max.z() }
            }};
        }

        /// <summary>
        /// Given a normal, returns the p-vertex (positive vertex) which is the corner point that lies furthest along it.
        /// Note that a normal component of 0 defaults to the max component value. This is opposite of the n-vertex which defaults to the min component value.
        /// </summary>
        /// <param name="normal"></param>
        /// <returns></returns>
        [[nodiscard]] constexpr vec3 pVertex(vec3 normal) const noexcept
        {
            return vec3{
                normal.x() >= 0.0f ? max.x() : min.x(),
                normal.y() >= 0.0f ? max.y() : min.y(),
                normal.z() >= 0.0f ? max.z() : min.z()
            };
        }

        /// <summary>
        /// Given a normal, returns the n-vertex (negative vertex) which is the corner point that lies furthest along the negative of the normal.
        /// Note that a normal component of 0 defaults to the min component value. This is opposite of the p-vertex which defaults to the max component value.
        /// </summary>
        /// <param name="normal"></param>
        /// <returns></returns>
        [[nodiscard]] constexpr vec3 nVertex(vec3 normal) const noexcept
        {
            return vec3{
                normal.x() >= 0.0f ? min.x() : max.x(),
                normal.y() >= 0.0f ? min.y() : max.y(),
                normal.z() >= 0.0f ? min.z() : max.z()
            };
        }

        /// <summary>
        /// Returns the closest point on/in the AABB to the provided point.
        /// If the point is inside the AABB then the point itself is returned.
        /// If the point is outside the AABB then the nearest point on the AABB surface is returned.
        /// </summary>
        /// <param name="point"></param>
        /// <returns></returns>
        [[nodiscard]] constexpr vec3 closestPoint(vec3 point) const noexcept
        {
            return clamp(point, min, max);
        }

        /// <summary>
        /// Returns the distance to the point. If the value is negative, the point is within the AABB.
        /// If the value is 0, then the point is on the AABB. If the value is positive, the point is outside the AABB.
        /// </summary>
        /// <param name="point"></param>
        /// <returns></returns>
        [[nodiscard]] constexpr float distanceTo(vec3 point) const noexcept
        {
            return distance(closestPoint(point), point);
        }

        /// <summary>
        /// Returns the squared distance to the point.
        /// </summary>
        /// <param name="point"></param>
        /// <returns></returns>
        [[nodiscard]] constexpr float distanceSqTo(vec3 point) const noexcept
        {
            return distanceSq(closestPoint(point), point);
        }

        /// <summary>
        /// Grows the AABB such that it encloses the other AABB.
        /// </summary>
        void grow(AABB const& other) noexcept
        {
            min = litl::min(min, other.min);
            max = litl::max(max, other.max);
        }

        /// <summary>
        /// Returns a new AABB which wraps this AABB transformed by the provided world matrix.
        /// </summary>
        [[nodiscard]] AABB transformed(mat4 worldMatrix) const noexcept
        {
            // Must transform all 8 corners and then calculate the new min/max.
            vec3 minPoint = vec3::max();
            vec3 maxPoint = vec3::min();

            std::array<vec3, 8> aabbCorners = corners();

            aabbCorners[0] = worldMatrix * aabbCorners[0];
            aabbCorners[1] = worldMatrix * aabbCorners[1];
            aabbCorners[2] = worldMatrix * aabbCorners[2];
            aabbCorners[3] = worldMatrix * aabbCorners[3];
            aabbCorners[4] = worldMatrix * aabbCorners[4];
            aabbCorners[5] = worldMatrix * aabbCorners[5];
            aabbCorners[6] = worldMatrix * aabbCorners[6];
            aabbCorners[7] = worldMatrix * aabbCorners[7];

            for (vec3& corner : aabbCorners)
            {
                minPoint = litl::min(minPoint, corner);
                maxPoint = litl::max(maxPoint, corner);
            }

            return fromMinMax(minPoint, maxPoint);
        }

        [[nodiscard]] static constexpr AABB fromMinMax(vec3 min, vec3 max) noexcept
        {
            return AABB{ .min = min, .max = max };
        }

        [[nodiscard]] static constexpr AABB fromCenterHalfExtents(vec3 center, vec3 halfExtents) noexcept
        {
            return AABB{ .min = (center - halfExtents), .max = (center + halfExtents) };
        }

        [[nodiscard]] static constexpr AABB fromPointRadius(vec3 center, float radius) noexcept
        {
            return fromCenterHalfExtents(center, vec3{ radius, radius, radius });
        }

        [[nodiscard]] static constexpr AABB fromPoints(std::span<vec3 const> points) noexcept
        {
            if (points.size() == 0)
            {
                return AABB{};
            }

            vec3 minPoint = vec3::max();
            vec3 maxPoint = vec3::min();

            for (auto point : points)
            {
                minPoint = litl::min(minPoint, point);
                maxPoint = litl::max(maxPoint, point);
            }

            return AABB{ .min = minPoint, .max = maxPoint };
        }
    };

    static_assert(std::is_trivially_copyable_v<AABB>);
}

#endif