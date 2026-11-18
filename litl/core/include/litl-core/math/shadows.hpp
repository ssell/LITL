#ifndef LITL_CORE_MATH_SHADOWS_H__
#define LITL_CORE_MATH_SHADOWS_H__

#include <array>
#include <span>

#include "litl-core/math/common.hpp"
#include "litl-core/math/types.hpp"
#include "litl-core/math/bounds.hpp"

namespace litl
{
    /// <summary>
    /// Practical split scheme (Zhang et al.). out[i] = far distance of cascade i.
    /// </summary>
    void computeCascadeSplits(float zNear, float zFar, uint32_t count, float lambda, std::span<float> out) noexcept
    {
        for (uint32_t i = 0u; i < count; ++i)
        {
            const float p = static_cast<float>(i + 1u) / static_cast<float>(count);
            const float logSplit = zNear * powf(zFar / zNear, p);
            const float uniSplit = zNear + ((zFar - zNear) * p);

            out[i] = lerp(uniSplit, logSplit, lambda);
        }
    }

    /// <summary>
    /// Corners of the camera frustum between view depths sliceNear and sliceFar.
    /// Built from the near corners alone, so it works for infinite-far projections.
    /// Perspective cameras only.
    /// </summary>
    [[nodiscard]] std::array<vec3, 8> sliceCornersPerspective(bounds::FrustumCorners const& corners, vec3 eye, float camNear, float sliceNear, float sliceFar) noexcept
    {
        const std::array<vec3, 4> rays{
           corners.nearLL - eye, corners.nearLR - eye,
           corners.nearUR - eye, corners.nearUL - eye
        };

        const float sN = sliceNear / camNear;
        const float sF = sliceFar / camNear;

        return {
            eye + rays[0] * sN, eye + rays[1] * sN, eye + rays[2] * sN, eye + rays[3] * sN,
            eye + rays[0] * sF, eye + rays[1] * sF, eye + rays[2] * sF, eye + rays[3] * sF
        };
    }

    /// <summary>
    /// Corners of the frustum between view depths sliceNear and sliceFar.
    /// Uses the near corners and side edges only: works for perspective, orthographic and infinite-far.
    /// </summary>
    [[nodiscard]] std::array<vec3, 8> sliceCornersOrthographic(bounds::Frustum const& frustum, float camNear, float sliceNear, float sliceFar) noexcept
    {
        using F = bounds::Frustum;

        const auto c = F::extractCorners(frustum);                 // near* are valid even when infinite
        const vec3 forward = frustum.getSide(F::Near).normal();

        const std::array<vec3, 4> nearCorners{ c.nearLL, c.nearLR, c.nearUR, c.nearUL };
        const std::array<vec3, 4> edges{
            frustum.edgeDirection(F::Left,  F::Bottom),
            frustum.edgeDirection(F::Right, F::Bottom),
            frustum.edgeDirection(F::Right, F::Top),
            frustum.edgeDirection(F::Left,  F::Top)
        };

        std::array<vec3, 8> out{};

        for (uint32_t i = 0u; i < 4u; ++i)
        {
            const float invDepthPerUnit = 1.0f / dot(edges[i], forward);
            out[i] = nearCorners[i] + edges[i] * ((sliceNear - camNear) * invDepthPerUnit);
            out[i + 4u] = nearCorners[i] + edges[i] * ((sliceFar - camNear) * invDepthPerUnit);
        }

        return out;
    }
}

#endif