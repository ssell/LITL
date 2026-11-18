#include <array>
#include <cmath>

#include "tests.hpp"
#include "litl-core/math.hpp"
#include "litl-core/math/shadows.hpp"

namespace litl::tests
{
    namespace
    {
        // Deliberately not axis-aligned so a swapped component or sign shows up.
        const vec3 s_eye{ 3.0f, 2.0f, -5.0f };
        const vec3 s_forward = normalize(vec3{ 1.0f, 0.0f, 1.0f });
        const vec3 s_worldUp{ 0.0f, 1.0f, 0.0f };

        constexpr float s_near = 0.1f;
        constexpr float s_far = 100.0f;
        constexpr float s_sliceNear = 5.0f;
        constexpr float s_sliceFar = 20.0f;

        mat4 makeView() noexcept
        {
            return mat4::lookAt(s_eye, s_eye + s_forward, s_worldUp);
        }

        // Matches glm::lookAtLH: s = normalize(cross(up, f)).
        vec3 viewRight() noexcept
        {
            return normalize(cross(s_worldUp, s_forward));
        }

        bounds::Frustum makeFrustum(mat4 const& proj, bool infiniteFar) noexcept
        {
            return bounds::Frustum::fromViewProjection(proj * makeView(), bounds::FrustumExtractionOptions{ .useInfiniteFar = infiniteFar });
        }

        float depthOf(vec3 const& p) noexcept
        {
            return dot(p - s_eye, s_forward);
        }

        void requireNear(float actual, float expected, float eps)
        {
            INFO("actual " << actual << ", expected " << expected << ", eps " << eps);
            REQUIRE(std::abs(actual - expected) <= eps);
        }

        void requireNear(vec3 const& actual, vec3 const& expected, float eps)
        {
            requireNear(actual.x(), expected.x(), eps);
            requireNear(actual.y(), expected.y(), eps);
            requireNear(actual.z(), expected.z(), eps);
        }

        // The perspective version scales by d / near (200x here), so tolerances scale with depth.
        constexpr float depthEps(float depth) noexcept
        {
            return 1e-4f * depth + 1e-5f;
        }
    }

    // -------------------------------------------------------------------------------------
    // sliceCornersPerspective
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("sliceCornersPerspective corners lie at slice depths", "[math::shadows]")
    {
        const auto proj = mat4::perspective(constants::pi * 0.5f, 2.0f, s_near, s_far);
        const auto frustum = makeFrustum(proj, true);
        const auto corners = bounds::Frustum::extractCorners(frustum);
        const auto slice = sliceCornersPerspective(corners, s_eye, s_near, s_sliceNear, s_sliceFar);

        for (uint32_t i = 0u; i < 4u; ++i)
        {
            requireNear(depthOf(slice[i]), s_sliceNear, depthEps(s_sliceNear));
            requireNear(depthOf(slice[i + 4u]), s_sliceFar, depthEps(s_sliceFar));
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("sliceCornersPerspective corners are collinear with the eye", "[math::shadows]")
    {
        const auto proj = mat4::perspective(constants::pi * 0.5f, 2.0f, s_near, s_far);
        const auto corners = bounds::Frustum::extractCorners(makeFrustum(proj, true));
        const auto slice = sliceCornersPerspective(corners, s_eye, s_near, s_sliceNear, s_sliceFar);

        const std::array<vec3, 4> nearCorners{ corners.nearLL, corners.nearLR, corners.nearUR, corners.nearUL };

        for (uint32_t i = 0u; i < 4u; ++i)
        {
            const vec3 rayNear = normalize(nearCorners[i] - s_eye);
            requireNear(normalize(slice[i] - s_eye), rayNear, 1e-4f);
            requireNear(normalize(slice[i + 4u] - s_eye), rayNear, 1e-4f);
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("sliceCornersPerspective extents match fov and aspect", "[math::shadows]")
    {
        // fovY 90 => half-height == depth. aspect 2 => half-width == 2 * depth.
        const auto proj = mat4::perspective(constants::pi * 0.5f, 2.0f, s_near, s_far);
        const auto corners = bounds::Frustum::extractCorners(makeFrustum(proj, true));
        const auto slice = sliceCornersPerspective(corners, s_eye, s_near, s_sliceNear, s_sliceFar);

        for (const float d : { s_sliceNear, s_sliceFar })
        {
            const uint32_t base = (d == s_sliceNear) ? 0u : 4u;

            requireNear((slice[base + 1u] - slice[base + 0u]).length(), 4.0f * d, depthEps(d));     // LL -> LR: width
            requireNear((slice[base + 3u] - slice[base + 0u]).length(), 2.0f * d, depthEps(d));     // LL -> UL: height
        }

        // Left corners are on the -right side, right corners on +right.
        REQUIRE(dot(slice[0] - s_eye, viewRight()) < 0.0f);
        REQUIRE(dot(slice[1] - s_eye, viewRight()) > 0.0f);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("sliceCornersPerspective infinite and finite frustums agree", "[math::shadows]")
    {
        const auto proj = mat4::perspective(constants::pi * 0.5f, 2.0f, s_near, s_far);

        const auto sliceInf = sliceCornersPerspective(bounds::Frustum::extractCorners(makeFrustum(proj, true)), s_eye, s_near, s_sliceNear, s_sliceFar);
        const auto sliceFin = sliceCornersPerspective(bounds::Frustum::extractCorners(makeFrustum(proj, false)), s_eye, s_near, s_sliceNear, s_sliceFar);

        for (uint32_t i = 0u; i < 8u; ++i)
        {
            requireNear(sliceInf[i], sliceFin[i], 1e-5f);
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("sliceCornersPerspective at camera near returns near corners", "[math::shadows]")
    {
        const auto proj = mat4::perspective(constants::pi * 0.5f, 2.0f, s_near, s_far);
        const auto corners = bounds::Frustum::extractCorners(makeFrustum(proj, true));
        const auto slice = sliceCornersPerspective(corners, s_eye, s_near, s_near, s_sliceFar);

        requireNear(slice[0], corners.nearLL, 1e-6f);
        requireNear(slice[1], corners.nearLR, 1e-6f);
        requireNear(slice[2], corners.nearUR, 1e-6f);
        requireNear(slice[3], corners.nearUL, 1e-6f);
    } LITL_END_TEST_CASE

        // -------------------------------------------------------------------------------------
        // sliceCornersOrthographic
        // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("sliceCornersOrthographic corners lie at slice depths", "[math::shadows]")
    {
        constexpr float orthoNear = 0.5f;
        const auto proj = mat4::orthographic(-4.0f, 4.0f, -2.0f, 2.0f, orthoNear, s_far);
        const auto slice = sliceCornersOrthographic(makeFrustum(proj, true), orthoNear, s_sliceNear, s_sliceFar);

        for (uint32_t i = 0u; i < 4u; ++i)
        {
            requireNear(depthOf(slice[i]), s_sliceNear, 1e-4f);
            requireNear(depthOf(slice[i + 4u]), s_sliceFar, 1e-4f);
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("sliceCornersOrthographic edges are parallel to forward", "[math::shadows]")
    {
        constexpr float orthoNear = 0.5f;
        const auto proj = mat4::orthographic(-4.0f, 4.0f, -2.0f, 2.0f, orthoNear, s_far);
        const auto slice = sliceCornersOrthographic(makeFrustum(proj, true), orthoNear, s_sliceNear, s_sliceFar);

        for (uint32_t i = 0u; i < 4u; ++i)
        {
            requireNear(slice[i + 4u] - slice[i], s_forward * (s_sliceFar - s_sliceNear), 1e-4f);
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("sliceCornersOrthographic extents are constant with depth", "[math::shadows]")
    {
        constexpr float orthoNear = 0.5f;
        const auto proj = mat4::orthographic(-4.0f, 4.0f, -2.0f, 2.0f, orthoNear, s_far);
        const auto slice = sliceCornersOrthographic(makeFrustum(proj, true), orthoNear, s_sliceNear, s_sliceFar);

        for (const uint32_t base : { 0u, 4u })
        {
            requireNear((slice[base + 1u] - slice[base + 0u]).length(), 8.0f, 1e-4f);
            requireNear((slice[base + 3u] - slice[base + 0u]).length(), 4.0f, 1e-4f);
        }

        requireNear(dot(slice[0] - s_eye, viewRight()), -4.0f, 1e-4f);
        requireNear(dot(slice[1] - s_eye, viewRight()), 4.0f, 1e-4f);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("sliceCornersOrthographic matches perspective version on a perspective frustum", "[math::shadows]")
    {
        // The edge-direction form is projection-agnostic. This pins that claim down.
        const auto proj = mat4::perspective(constants::pi * 0.5f, 2.0f, s_near, s_far);
        const auto frustum = makeFrustum(proj, true);

        const auto viaRays = sliceCornersPerspective(bounds::Frustum::extractCorners(frustum), s_eye, s_near, s_sliceNear, s_sliceFar);
        const auto viaEdges = sliceCornersOrthographic(frustum, s_near, s_sliceNear, s_sliceFar);

        for (uint32_t i = 0u; i < 8u; ++i)
        {
            requireNear(viaEdges[i], viaRays[i], depthEps(s_sliceFar) * 10.0f);
        }
    } LITL_END_TEST_CASE
}