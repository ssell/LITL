#ifndef LITL_MATH_BOUNDS_SPHERE_H__
#define LITL_MATH_BOUNDS_SPHERE_H__

#include <type_traits>

#include "litl-core/assert.hpp"
#include "litl-core/math/types/vec3.hpp"

namespace litl::bounds
{
    struct Sphere
    {
        vec3 center{ 0.0f, 0.0f, 0.0f };
        float radius{ 1.0f };

        [[nodiscard]] static constexpr Sphere fromCenterRadius(vec3 center, float radius) noexcept
        {
            LITL_ASSERT_MSG(radius > 0.0f, "Sphere radius must be > 0", Sphere{});
            return Sphere{ .center = center, .radius = radius };
        }

        [[nodiscard]] static constexpr Sphere fromPoints(std::span<vec3 const> points) noexcept
        {
            if (points.size() == 0)
            {
                return Sphere{};
            }

            vec3 minPoint = vec3::max();
            vec3 maxPoint = vec3::min();

            for (auto point : points)
            {
                minPoint = litl::min(minPoint, point);
                maxPoint = litl::max(maxPoint, point);
            }

            float radius = (minPoint.distanceTo(maxPoint) * 0.5f);

            return Sphere{ .center = (minPoint + radius), .radius = radius };
        }

        /// <summary>
        /// Centroid sphere with a quantized radius. A stable radius keeps the ortho extents,
        /// and therefore the world-space texel size, constant while the camera rotates.
        /// </summary>
        [[nodiscard]] static constexpr bounds::Sphere fromCorners(std::span<vec3 const, 8> points) noexcept
        {
            vec3 center{ 0.0f, 0.0f, 0.0f };

            for (const auto& p : points)
            {
                center = center + p;
            }

            center = center * (1.0f / 8.0f);

            float radius = 0.0f;

            for (const auto& p : points)
            {
                radius = std::max(radius, (p - center).length());
            }

            radius = std::ceil(radius * 16.0f) / 16.0f;

            return bounds::Sphere{ .center = center, .radius = radius };
        }
    };

    static_assert(std::is_trivially_copyable_v<Sphere>);
}

#endif