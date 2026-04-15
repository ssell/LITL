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

        [[nodiscard]] constexpr vec3 extents() const noexcept
        {
            return (max - min);
        }

        [[nodiscard]] constexpr vec3 center() const noexcept
        {
            return (min + ((max - min) * 0.5f));
        }

        static constexpr AABB fromMinMax(vec3 min, vec3 max) noexcept
        {
            return AABB{ .min = min, .max = max };
        }

        static constexpr AABB fromCenterHalfExtents(vec3 center, vec3 halfExtents) noexcept
        {
            return AABB{ .min = (center - halfExtents), .max = (center + halfExtents) };
        }

        static constexpr AABB fromPoints(std::span<vec3 const> points) noexcept
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