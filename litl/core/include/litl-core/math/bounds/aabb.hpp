#ifndef LITL_MATH_BOUNDS_AABB_H__
#define LITL_MATH_BOUNDS_AABB_H__

#include <span>

#include "litl-core/assert.hpp"
#include "litl-core/math/types/vec3.hpp"

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

        /// <summary>
        /// Given a normal, returns the p-vertex (positive vertex) which is the corner point that lies furthest along it.
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
        /// </summary>
        /// <param name="normal"></param>
        /// <returns></returns>
        [[nodiscard]] constexpr vec3 nVertex(vec3 normal) const noexcept
        {
            return pVertex(-normal);
        }

        [[nodiscard]] static constexpr AABB fromMinMax(vec3 min, vec3 max) noexcept
        {
            return AABB{ .min = min, .max = max };
        }

        [[nodiscard]] static constexpr AABB fromCenterHalfExtents(vec3 center, vec3 halfExtents) noexcept
        {
            return AABB{ .min = (center - halfExtents), .max = (center + halfExtents) };
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
}

#endif