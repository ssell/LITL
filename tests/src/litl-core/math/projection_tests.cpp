#include "tests.hpp"
#include "litl-core/math/projection.hpp"
#include "litl-core/math/traits.hpp"

namespace litl::tests
{
    namespace
    {
        constexpr float s_fovy = Traits<float>::pi * 0.5f;
        constexpr float s_aspect = 2.0f;
        constexpr float s_near = 0.1f;
        constexpr float s_far = 100.0f;

        const vec2 kScreen{ 800.0f, 400.0f };

        mat4 makeProj() noexcept
        {
            return mat4::perspective(s_fovy, s_aspect, s_near, s_far);
        }

        // Camera at the origin looking down +Z (left-handed), +Y up.
        mat4 makeView() noexcept
        {
            return mat4::lookAt(vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.0f, 0.0f, 1.0f }, vec3{ 0.0f, 1.0f, 0.0f });
        }

        void requireApprox(vec3 const& actual, vec3 const& expected, float eps = Traits<float>::epsilon)
        {
            REQUIRE(fequals(actual.x(), expected.x()));
            REQUIRE(fequals(actual.y(), expected.y()));
            REQUIRE(fequals(actual.z(), expected.z()));
        }

        // Projects a world position back to screen pixels using the same conventions screenToWorldSpace assumes.
        vec2 worldToScreen(vec3 const& world, vec2 screenSize, mat4 const& view, mat4 const& proj)
        {
            const vec4 clip = (proj * view) * vec4{ world.x(), world.y(), world.z(), 1.0f };
            const float ndcX = clip.x() / clip.w();
            const float ndcY = clip.y() / clip.w();

            return vec2{ (ndcX + 1.0f) * 0.5f * screenSize.x(), (ndcY + 1.0f) * 0.5f * screenSize.y() };
        }
    }

    // -------------------------------------------------------------------------------------
    // screenToNdcSpace
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("screenToNdcSpace maps top-left to (-1, -1)", "[math::projection]")
    {
        const vec2 ndc = screenToNdcSpace(vec2{ 0.0f, 0.0f }, kScreen);
        REQUIRE(fequals(ndc.x(), -1.0f));
        REQUIRE(fequals(ndc.y(), -1.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("screenToNdcSpace maps bottom-right to (1, 1)", "[math::projection]")
    {
        const vec2 ndc = screenToNdcSpace(kScreen, kScreen);
        REQUIRE(isOne(ndc.x()));
        REQUIRE(isOne(ndc.y()));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("screenToNdcSpace maps centre to (0, 0)", "[math::projection]")
    {
        const vec2 ndc = screenToNdcSpace(vec2{ 400.0f, 200.0f }, kScreen);
        REQUIRE(isZero(ndc.x()));
        REQUIRE(isZero(ndc.y()));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("screenToNdcSpace handles non-square sizes per axis", "[math::projection]")
    {
        // Quarter across, three quarters down.
        const vec2 ndc = screenToNdcSpace(vec2{ 200.0f, 300.0f }, kScreen);
        REQUIRE(fequals(ndc.x(), -0.5f));
        REQUIRE(fequals(ndc.y(), 0.5f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("screenToNdcSpace asserts on zero-sized screen", "[math::projection]")
    {
        LITL_START_ASSERT_CAPTURE
            const vec2 ndc = screenToNdcSpace(vec2{ 10.0f, 10.0f }, vec2{ 0.0f, 100.0f });
        REQUIRE(ndc == vec2{});
        LITL_END_ASSERT_CAPTURE

            LITL_START_ASSERT_CAPTURE
            const vec2 ndc = screenToNdcSpace(vec2{ 10.0f, 10.0f }, vec2{ 100.0f, 0.0f });
        REQUIRE(ndc == vec2{});
        LITL_END_ASSERT_CAPTURE
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // screenToWorldSpace
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("screenToWorldSpace centre ray points along camera forward", "[math::projection]")
    {
        const Ray ray = screenToWorldSpace(vec2{ 400.0f, 200.0f }, kScreen, makeView(), makeProj());

        requireApprox(ray.direction, vec3{ 0.0f, 0.0f, 1.0f });
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("screenToWorldSpace origin lies on the near plane", "[math::projection]")
    {
        // The engine uses reversed-Z (NDC z: 1 = near, 0 = far). Catches the near/far being swapped.
        const Ray ray = screenToWorldSpace(vec2{ 400.0f, 200.0f }, kScreen, makeView(), makeProj());

        requireApprox(ray.origin, vec3{ 0.0f, 0.0f, s_near });
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("screenToWorldSpace direction is normalized", "[math::projection]")
    {
        const vec2 points[] = { { 0.0f, 0.0f }, { 800.0f, 0.0f }, { 0.0f, 400.0f }, { 800.0f, 400.0f }, { 123.0f, 321.0f } };

        for (vec2 const& p : points)
        {
            const Ray ray = screenToWorldSpace(p, kScreen, makeView(), makeProj());
            REQUIRE(isOne(ray.direction.length()));
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("screenToWorldSpace corners match frustum edges", "[math::projection]")
    {
        // fovY = 90deg => tan(fovY/2) = 1, so at z = 1 the frustum spans x in [-aspect, aspect], y in [-1, 1].
        // Screen Y grows downward, so screen y == 0 is the top of the view (+Y in world space).
        const mat4 view = makeView();
        const mat4 proj = makeProj();

        const struct { vec2 screen; vec3 expected; } cases[] =
        {
            { vec2{   0.0f,   0.0f }, vec3{ -s_aspect,  1.0f, 1.0f } },   // top-left
            { vec2{ 800.0f,   0.0f }, vec3{  s_aspect,  1.0f, 1.0f } },   // top-right
            { vec2{   0.0f, 400.0f }, vec3{ -s_aspect, -1.0f, 1.0f } },   // bottom-left
            { vec2{ 800.0f, 400.0f }, vec3{  s_aspect, -1.0f, 1.0f } },   // bottom-right
        };

        for (auto const& c : cases)
        {
            const Ray ray = screenToWorldSpace(c.screen, kScreen, view, proj);
            requireApprox(ray.direction, c.expected.normalized());
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("screenToWorldSpace respects camera position and orientation", "[math::projection]")
    {
        // Camera at (10, 5, -3) looking down +X.
        const vec3 camPos{ 10.0f, 5.0f, -3.0f };
        const mat4 view = mat4::lookAt(camPos, camPos + vec3{ 1.0f, 0.0f, 0.0f }, vec3{ 0.0f, 1.0f, 0.0f });
        const mat4 proj = makeProj();

        const Ray ray = screenToWorldSpace(vec2{ 400.0f, 200.0f }, kScreen, view, proj);

        requireApprox(ray.direction, vec3{ 1.0f, 0.0f, 0.0f });
        requireApprox(ray.origin, camPos + vec3{ s_near, 0.0f, 0.0f });
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("screenToWorldSpace round trips back to the same pixel", "[math::projection]")
    {
        const mat4 view = mat4::lookAt(vec3{ 2.0f, 3.0f, -4.0f }, vec3{ 0.0f, 1.0f, 5.0f }, vec3{ 0.0f, 1.0f, 0.0f });
        const mat4 proj = makeProj();

        const vec2 points[] = { { 0.0f, 0.0f }, { 800.0f, 400.0f }, { 400.0f, 200.0f }, { 123.0f, 321.0f }, { 700.0f, 50.0f } };
        const float distances[] = { 1.0f, 10.0f, 50.0f };

        for (vec2 const& p : points)
        {
            const Ray ray = screenToWorldSpace(p, kScreen, view, proj);

            for (float t : distances)
            {
                const vec2 back = worldToScreen(ray.origin + (ray.direction * t), kScreen, view, proj);
                const auto bx = back.x();
                const auto by = back.y();
                const auto px = p.x();
                const auto py = p.y();
                REQUIRE(fequals(back.x(), p.x(), Traits<float>::relative_epsilon, 0.05f));
                REQUIRE(fequals(back.y(), p.y(), Traits<float>::relative_epsilon, 0.05f));
            }
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("screenToWorldSpace asserts on zero-sized screen", "[math::projection]")
    {
        LITL_START_ASSERT_CAPTURE
            const Ray ray = screenToWorldSpace(vec2{ 10.0f, 10.0f }, vec2{ 0.0f, 0.0f }, makeView(), makeProj());
            (void)ray;
        LITL_END_ASSERT_CAPTURE
    } LITL_END_TEST_CASE
}