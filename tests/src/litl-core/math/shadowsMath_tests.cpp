#include <array>
#include <cmath>

#include "tests.hpp"
#include "litl-core/math.hpp"
#include "litl-core/math/shadows.hpp"
#include "litl-engine/constants.hpp"

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

        vec3 forwardFromYawPitch(float yawDeg, float pitchDeg) noexcept
        {
            const float yaw = yawDeg * (constants::pi / 180.0f);
            const float pitch = pitchDeg * (constants::pi / 180.0f);
            return normalize(vec3{ std::cos(pitch) * std::sin(yaw), std::sin(pitch), std::cos(pitch) * std::cos(yaw) });
        }

        bounds::Sphere sliceSphere(vec3 const& forward, float sliceNear, float sliceFar) noexcept
        {
            const auto aspectRatio = 16.0f / 9.0f;
            const auto fovY = constants::pi / 3.0f;
            const auto tanY = std::tan(fovY * 0.5f);
            const auto tanX = aspectRatio * tanY;
            const auto view = mat4::lookAt(s_eye, s_eye + forward, s_worldUp);
            const auto proj = mat4::perspective(fovY, aspectRatio, s_near, s_far);
            const auto frustum = bounds::Frustum::fromViewProjection(proj * view, bounds::FrustumExtractionOptions{ .useInfiniteFar = true });
            
            return sliceSpherePerspective(s_eye, forward, tanX, tanY, sliceNear, sliceFar);
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

    LITL_TEST_CASE("computeCascadeSplits is strictly increasing and ends at far", "[math::shadows]")
    {
        for (const float lambda : { 0.0f, 0.5f, 0.75f, 1.0f })
        {
            for (uint32_t count = 1u; count <= constants::MaxShadowCascades; ++count)
            {
                INFO("lambda " << lambda << ", count " << count);

                std::array<float, 4> splits{};
                computeCascadeSplits(s_near, s_far, count, lambda, splits);

                float previous = s_near;

                for (uint32_t i = 0u; i < count; ++i)
                {
                    INFO("split " << i << " = " << splits[i] << ", previous " << previous);
                    REQUIRE(splits[i] > previous);
                    previous = splits[i];
                }

                requireNear(splits[count - 1u], s_far, 1e-5f * s_far);
            }
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("computeCascadeSplits lambda endpoints are uniform and logarithmic", "[math::shadows]")
    {
        constexpr uint32_t count = 4u;
        std::array<float, count> uniform{};
        std::array<float, count> logarithmic{};

        computeCascadeSplits(s_near, s_far, count, 0.0f, uniform);
        computeCascadeSplits(s_near, s_far, count, 1.0f, logarithmic);

        const float step = (s_far - s_near) / static_cast<float>(count);
        const float ratio = std::pow(s_far / s_near, 1.0f / static_cast<float>(count));

        for (uint32_t i = 0u; i < count; ++i)
        {
            requireNear(uniform[i], s_near + (step * static_cast<float>(i + 1u)), 1e-4f);
        }

        // Logarithmic: each split is a constant multiple of the previous one (s_near for i == 0).
        float previous = s_near;

        for (uint32_t i = 0u; i < count; ++i)
        {
            requireNear(logarithmic[i] / previous, ratio, 1e-4f);
            previous = logarithmic[i];
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("computeCascadeSplits writes only count entries", "[math::shadows]")
    {
        // The shader's cascade search relied on unused splits being 0. This pins down that the
        // function itself never touches them, so the zeros come only from ShadowRenderViewData{}.
        std::array<float, 4> splits{ -1.0f, -1.0f, -1.0f, -1.0f };
        computeCascadeSplits(s_near, s_far, 2u, 0.75f, splits);

        REQUIRE(splits[2] == -1.0f);
        REQUIRE(splits[3] == -1.0f);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Slice sphere radius is invariant under camera rotation", "[math::shadows]")
    {
        // A changing radius changes the ortho extents, which changes world texel size,
        // which shows up as shimmering shadow edges while the camera turns.
        std::array<float, 4> splits{};
        computeCascadeSplits(s_near, s_far, 4u, 0.75f, splits);

        float sliceNear = s_near;

        for (uint32_t c = 0u; c < 4u; ++c)
        {
            const float baseline = sliceSphere(forwardFromYawPitch(0.0f, 0.0f), sliceNear, splits[c]).radius;

            // Pitch stays clear of +-90 where lookAt with a +Y up degenerates.
            for (float pitch = -80.0f; pitch <= 80.0f; pitch += 10.0f)
            {
                for (float yaw = 0.0f; yaw < 360.0f; yaw += 7.0f)
                {
                    const float radius = sliceSphere(forwardFromYawPitch(yaw, pitch), sliceNear, splits[c]).radius;
                    REQUIRE(radius == baseline);
                }
            }

            sliceNear = splits[c];
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("sliceSpherePerspective contains every slice corner", "[math::shadows]")
    {
        constexpr float fovY = constants::pi / 3.0f;
        constexpr float aspect = 16.0f / 9.0f;
        const float tanY = std::tan(fovY * 0.5f);
        const float tanX = aspect * tanY;

        const auto proj = mat4::perspective(fovY, aspect, s_near, s_far);

        std::array<float, 4> splits{};
        computeCascadeSplits(s_near, s_far, 4u, 0.75f, splits);

        // Pitch stays clear of +-90 where lookAt with a +Y up degenerates.
        for (float pitch = -80.0f; pitch <= 80.0f; pitch += 10.0f)
        {
            for (float yaw = 0.0f; yaw < 360.0f; yaw += 7.0f)
            {
                const vec3 forward = forwardFromYawPitch(yaw, pitch);
                const auto view = mat4::lookAt(s_eye, s_eye + forward, s_worldUp);
                const auto frustum = bounds::Frustum::fromViewProjection(proj * view, bounds::FrustumExtractionOptions{ .useInfiniteFar = true });
                const auto corners = bounds::Frustum::extractCorners(frustum);

                float sliceNear = s_near;

                for (uint32_t c = 0u; c < 4u; ++c)
                {
                    const auto sphere = sliceSpherePerspective(s_eye, forward, tanX, tanY, sliceNear, splits[c]);
                    const auto slice = sliceCornersPerspective(corners, s_eye, s_near, sliceNear, splits[c]);

                    // The corners are the noisy side (scaled by d / near), so the tolerance follows depth.
                    const float eps = depthEps(splits[c]) * 10.0f;

                    for (uint32_t i = 0u; i < 8u; ++i)
                    {
                        const float dist = (slice[i] - sphere.center).length();

                        INFO("yaw " << yaw << ", pitch " << pitch << ", cascade " << c << ", corner " << i
                            << ", dist " << dist << ", radius " << sphere.radius << ", eps " << eps);
                        REQUIRE(dist <= sphere.radius + eps);
                    }

                    sliceNear = splits[c];
                }
            }
        }
    } LITL_END_TEST_CASE
}