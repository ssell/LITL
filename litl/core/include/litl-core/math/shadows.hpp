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
    /// World-space corners of the camera frustum between two view distances.
    /// Each frustum edge is a straight line on which view depth is linear, so a plain lerp is exact.
    /// </summary>
    [[nodiscard]] std::array<vec3, 8> sliceCorners(bounds::FrustumCorners const& corners, float camNear, float camFar, float sliceNear, float sliceFar) noexcept
    {
        const float tN = (sliceNear - camNear) / (camFar - camNear);
        const float tF = (sliceFar - camNear) / (camFar - camNear);

        const auto at = [](vec3 const& n, vec3 const& f, float t) { return n + ((f - n) * t); };

        return {
            at(corners.nearLL, corners.farLL, tN), at(corners.nearLR, corners.farLR, tN), at(corners.nearUR, corners.farUR, tN), at(corners.nearUL, corners.farUL, tN),
            at(corners.nearLL, corners.farLL, tF), at(corners.nearLR, corners.farLR, tF), at(corners.nearUR, corners.farUR, tF), at(corners.nearUL, corners.farUL, tF)
        };
    }
}

#endif