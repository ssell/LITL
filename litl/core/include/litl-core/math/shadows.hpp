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
    void computeCascadeSplits(float zNear, float zFar, uint32_t count, float lambda, std::span<float> out) noexcept;

    /// <summary>
    /// Corners of the camera frustum between view depths sliceNear and sliceFar.
    /// Built from the near corners alone, so it works for infinite-far projections.
    /// Perspective cameras only.
    /// </summary>
    [[nodiscard]] std::array<vec3, 8> sliceCornersPerspective(bounds::FrustumCorners const& corners, vec3 eye, float camNear, float sliceNear, float sliceFar) noexcept;

    /// <summary>
    /// Corners of the frustum between view depths sliceNear and sliceFar.
    /// Uses the near corners and side edges only: works for perspective, orthographic and infinite-far.
    /// </summary>
    [[nodiscard]] std::array<vec3, 8> sliceCornersOrthographic(bounds::Frustum const& frustum, float camNear, float sliceNear, float sliceFar) noexcept;

    /// <summary>
    /// Tight bounding sphere of the perspective frustum slice between view depths sliceNear and sliceFar.
    /// Computed analytically from the projection, so it does not depend on camera orientation.
    /// Symmetric frustums only. The radius is quantized the same way as bounds::Sphere::fromCorners.
    /// </summary>
    [[nodiscard]] bounds::Sphere sliceSpherePerspective(vec3 eye, vec3 forward, float tanHalfFovX, float tanHalfFovY, float sliceNear, float sliceFar) noexcept;
}

#endif