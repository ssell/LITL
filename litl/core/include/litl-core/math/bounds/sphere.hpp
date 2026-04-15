#ifndef LITL_MATH_BOUNDS_SPHERE_H__
#define LITL_MATH_BOUNDS_SPHERE_H__

#include "litl-core/math/types/vec3.hpp"

namespace litl::bounds
{
    struct Sphere
    {
        vec3 center{ 0.0f, 0.0f, 0.0f };
        float radius{ 1.0f };

        static constexpr Sphere fromCenterRadius(vec3 center, float radius) noexcept
        {
            return Sphere{ .center = center, .radius = radius };
        }

        static constexpr Sphere fromPoints(std::span<vec3 const> points) noexcept
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
    };
}

#endif