#include <algorithm>
#include <array>

#include "tests.hpp"
#include "litl-core/math.hpp"

namespace litl::tests
{
    // -------------------------------------------------------------------------------------
    // AABB
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("aabb from min max", "[math::bounds]")
    {
        constexpr auto expectedMin = vec3{ -5.0f, -5.0f, -5.0f };
        constexpr auto expectedMax = vec3{ 5.0f, 5.0f, 5.0f };

        auto aabb = bounds::AABB::fromMinMax(expectedMin, expectedMax);

        REQUIRE(aabb.min == expectedMin);
        REQUIRE(aabb.max == expectedMax);
        REQUIRE(aabb.center() == vec3{ 0.0f, 0.0f, 0.0f });
        REQUIRE(aabb.extents() == vec3{ 10.0f, 10.0f, 10.0f });
        REQUIRE(aabb.halfExtents() == vec3{ 5.0f, 5.0f, 5.0f });
    } LITL_END_TEST_CASE
        

    LITL_TEST_CASE("aabb center half-extents", "[math::bounds]")
    {
        auto aabb = bounds::AABB::fromCenterHalfExtents(vec3{ 5.0f, 5.0f, 5.0f }, vec3{ 5.0f, 5.0f, 5.0f });

        constexpr auto expectedMin = vec3{ 0.0f, 0.0f, 0.0f };
        constexpr auto expectedMax = vec3{ 10.0f, 10.0f, 10.0f };

        REQUIRE(aabb.min == expectedMin);
        REQUIRE(aabb.max == expectedMax);
        REQUIRE(aabb.center() == vec3{ 5.0f, 5.0f, 5.0f });
        REQUIRE(aabb.extents() == vec3{ 10.0f, 10.0f, 10.0f });
        REQUIRE(aabb.halfExtents() == vec3{ 5.0f, 5.0f, 5.0f });
    } LITL_END_TEST_CASE
        

    LITL_TEST_CASE("aabb from points", "[math::bounds]")
    {
        std::array<vec3, 5> points{
            vec3{0.0f, 0.0f, 0.0f},
            vec3{10.0f, 0.0f, 0.0f},
            vec3{0.0f, 10.0f, 0.0f},
            vec3{0.0f, 0.0f, 10.0f},
            vec3{0.0f, -100.0f, 0.0f},
        };

        auto aabb = bounds::AABB::fromPoints(points);

        constexpr auto expectedMin = vec3{ 0.0f, -100.0f, 0.0f };
        constexpr auto expectedMax = vec3{ 10.0f, 10.0f, 10.0f };

        REQUIRE(aabb.min == expectedMin);
        REQUIRE(aabb.max == expectedMax);

        std::array<vec3, 1> point{
            vec3{0.0f, 10.0f, 0.0f}
        };

        aabb = bounds::AABB::fromPoints(point);

        REQUIRE(aabb.min == point[0]);
        REQUIRE(aabb.max == point[0]);
        REQUIRE(aabb.center() == point[0]);
        REQUIRE(aabb.extents() == vec3{ 0.0f, 0.0f, 0.0f });
        REQUIRE(aabb.halfExtents() == vec3{ 0.0f, 0.0f, 0.0f });
    } LITL_END_TEST_CASE
        

    LITL_TEST_CASE("aabb contains point", "[math::bounds]")
    {
        auto aabb = bounds::AABB::fromCenterHalfExtents(vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 5.0f, 5.0f, 5.0f });

        REQUIRE(bounds::contains(aabb, vec3{ 0.0f, 0.0f, 0.0f })    == true);
        REQUIRE(bounds::contains(aabb, vec3{ 5.01f, 0.0f, 0.0f })   == false);
        REQUIRE(bounds::contains(aabb, vec3{ 0.0f, 5.01f, 0.0f })   == false);
        REQUIRE(bounds::contains(aabb, vec3{ 0.0f, 0.0f, 5.01f })   == false);
        REQUIRE(bounds::contains(aabb, vec3{ 5.0f, 0.0f, 0.0f })    == true);
        REQUIRE(bounds::contains(aabb, vec3{ 0.0f, 5.0f, 0.0f })    == true);
        REQUIRE(bounds::contains(aabb, vec3{ 0.0f, 0.0f, 5.0f })    == true);
        REQUIRE(bounds::contains(aabb, vec3{ 5.0f, 5.0f, 0.0f })    == true);
        REQUIRE(bounds::contains(aabb, vec3{ 0.0f, 5.0f, 5.0f })    == true);
        REQUIRE(bounds::contains(aabb, vec3{ 5.0f, 0.0f, 5.0f })    == true);
        REQUIRE(bounds::contains(aabb, vec3{ 5.0f, 5.0f, 5.0f })    == true);
        REQUIRE(bounds::contains(aabb, vec3{ -5.01f, 0.0f, 0.0f })  == false);
        REQUIRE(bounds::contains(aabb, vec3{ 0.0f, -5.01f, 0.0f })  == false);
        REQUIRE(bounds::contains(aabb, vec3{ 0.0f, 0.0f, -5.01f })  == false);
        REQUIRE(bounds::contains(aabb, vec3{ -5.0f, 0.0f, 0.0f })   == true);
        REQUIRE(bounds::contains(aabb, vec3{ 0.0f, -5.0f, 0.0f })   == true);
        REQUIRE(bounds::contains(aabb, vec3{ 0.0f, 0.0f, -5.0f })   == true);
        REQUIRE(bounds::contains(aabb, vec3{ -5.0f, -5.0f, 0.0f })  == true);
        REQUIRE(bounds::contains(aabb, vec3{ 0.0f, -5.0f, -5.0f })  == true);
        REQUIRE(bounds::contains(aabb, vec3{ -5.0f, 0.0f, -5.0f })  == true);
        REQUIRE(bounds::contains(aabb, vec3{ -5.0f, -5.0f, -5.0f }) == true);
    } LITL_END_TEST_CASE
        
    // -------------------------------------------------------------------------------------
    // Sphere
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("sphere from center radius", "[math::bounds]")
    {
        bounds::Sphere sphere = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 5.0f, 0.0f }, 5.0f);

        REQUIRE(sphere.center == vec3{ 0.0f, 5.0f, 0.0f });
        REQUIRE(fequals(sphere.radius, 5.0f));
    } LITL_END_TEST_CASE


    LITL_TEST_CASE("sphere from points", "[math::bounds]")
    {
        std::array<vec3, 5> points{
            vec3{0.0f, 0.0f, 0.0f},
            vec3{10.0f, 0.0f, 0.0f},
            vec3{0.0f, 10.0f, 0.0f},
            vec3{0.0f, 0.0f, 10.0f},
            vec3{0.0f, -100.0f, 0.0f},
        };

        auto sphere = bounds::Sphere::fromPoints(points);

        constexpr auto expectedCenter = vec3{ 55.4526825f, -44.5473175f, 55.4526825 };
        constexpr auto expectedRadius = 55.4526825f;

        REQUIRE(sphere.center == expectedCenter);
        REQUIRE(fequals(sphere.radius, expectedRadius));

        std::array<vec3, 1> point{
            vec3{0.0f, 10.0f, 0.0f}
        };

        sphere = bounds::Sphere::fromPoints(point);

        REQUIRE(sphere.center == point[0]);
        REQUIRE(isZero(sphere.radius));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("sphere contains point", "[math::bounds]")
    {
        bounds::Sphere sphere{ vec3{ 0.0f, 5.0f, 0.0f }, 5.0f };

        REQUIRE(bounds::contains(sphere, vec3{ 0.0f, 0.0f, 0.0f }) == true);
        REQUIRE(bounds::contains(sphere, vec3{ 0.0f, 5.0f, 0.0f }) == true);
        REQUIRE(bounds::contains(sphere, vec3{ 0.0f, 10.0f, 0.0f }) == true);
        REQUIRE(bounds::contains(sphere, vec3{ 0.0f, 10.1f, 0.0f }) == false);
        REQUIRE(bounds::contains(sphere, vec3{ -5.0f, 5.0f, 0.0f }) == true);
        REQUIRE(bounds::contains(sphere, vec3{ 5.0f, 5.0f, 0.0f }) == true);
        REQUIRE(bounds::contains(sphere, vec3{ -5.1f, 5.0f, 0.0f }) == false);
        REQUIRE(bounds::contains(sphere, vec3{ 5.1f, 5.0f, 0.0f }) == false);
        REQUIRE(bounds::contains(sphere, vec3{ 0.0f, 5.0f, -5.0f }) == true);
        REQUIRE(bounds::contains(sphere, vec3{ 0.0f, 5.0f, 5.0f }) == true);
        REQUIRE(bounds::contains(sphere, vec3{ 0.0f, 5.0f, -5.1f }) == false);
        REQUIRE(bounds::contains(sphere, vec3{ 0.0f, 5.0f, 5.1f }) == false);
    } LITL_END_TEST_CASE
        
    // -------------------------------------------------------------------------------------
    // Plane
    // -------------------------------------------------------------------------------------
        
    LITL_TEST_CASE("plane from point normal", "[math::bounds]")
    {
        auto planeY = bounds::Plane::fromPointNormal(vec3{ 0.0f, 10.0f, 0.0f }, vec3::up());
        auto planeYNeg = bounds::Plane::fromPointNormal(vec3{ 0.0f, -10.0f, 0.0f }, -vec3::up());

        REQUIRE(planeY.normal() == vec3::up());
        REQUIRE(fequals(planeY.d(), 10.0f));

        REQUIRE(planeYNeg.normal() == -vec3::up());
        REQUIRE(fequals(planeY.d(), 10.0f));
    } LITL_END_TEST_CASE
        
    LITL_TEST_CASE("plane from triangle", "[math::bounds]")
    {
        /**
         *              c
         *             /|
         *           /  |
         *         /    |
         *       /      |
         *     /        |
         *   /          |
         *  a --------- b
         */
        const vec3 a = vec3{ 0.0f, 0.0f, 0.0f };
        const vec3 b = vec3{ 1.0f, 0.0f, 0.0f };
        const vec3 c = vec3{ 1.0f, 1.0f, 0.0f };

        auto planeCCW = bounds::Plane::fromTriangleCCW(a, b, c);
        auto planeCW  = bounds::Plane::fromTriangleCW(a, c, b);

        REQUIRE(planeCCW.normal() == vec3::forward());
        REQUIRE(fequals(planeCCW.d(), 0.0f));

        REQUIRE(planeCW.normal() == vec3::forward());
        REQUIRE(fequals(planeCW.d(), 0.0f));
    } LITL_END_TEST_CASE
        
    LITL_TEST_CASE("plane signed distance", "[math::bounds]")
    {
        auto plane = bounds::Plane::fromPointNormal(vec3{ 0.0f, 0.0f, 0.0f }, vec3::up());

        REQUIRE(fequals(plane.signedDistance(vec3{ 0.0f, 0.0f, 0.0f }), 0.0f));
        REQUIRE(fequals(plane.signedDistance(vec3{ 0.0f, 1.0f, 0.0f }), 1.0f));
        REQUIRE(fequals(plane.signedDistance(vec3{ 0.0f, 10.0f, 0.0f }), 10.0f));
        REQUIRE(fequals(plane.signedDistance(vec3{ 0.0f, -1.0f, 0.0f }), -1.0f));
        REQUIRE(fequals(plane.signedDistance(vec3{ 0.0f, -10.0f, 0.0f }), -10.0f));

        REQUIRE(fequals(plane.signedDistance(vec3{ 100.0f, 0.0f, -100.0f }), 0.0f));
        REQUIRE(fequals(plane.signedDistance(vec3{ 100.0f, 1.0f, -100.0f }), 1.0f));
        REQUIRE(fequals(plane.signedDistance(vec3{ 100.0f, 10.0f, -100.0f }), 10.0f));
        REQUIRE(fequals(plane.signedDistance(vec3{ 100.0f, -1.0f, -100.0f }), -1.0f));
        REQUIRE(fequals(plane.signedDistance(vec3{ 100.0f, -10.0f, -100.0f }), -10.0f));
    } LITL_END_TEST_CASE
        
    LITL_TEST_CASE("plane contains point", "[math::bounds]")
    {
        // plane normal points inward
        auto plane = bounds::Plane::fromPointNormal(vec3{ 0.0f, 0.0f, 0.0f }, vec3::up());

        REQUIRE(bounds::contains(plane, vec3{ 0.0f, 0.0f, 0.0f }) == true);
        REQUIRE(bounds::contains(plane, vec3{ 0.0f, 0.01f, 0.0f }) == true);
        REQUIRE(bounds::contains(plane, vec3{ 0.0f, -0.01f, 0.0f }) == false);

        REQUIRE(bounds::contains(plane, vec3{ 100.0f, 0.0f, -100.0f }) == true);
        REQUIRE(bounds::contains(plane, vec3{ 100.0f, 0.01f, -100.0f }) == true);
        REQUIRE(bounds::contains(plane, vec3{ 100.0f, -0.01f, -100.0f }) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("plane d is signed distance from origin", "[math::bounds]")
    {
        auto plane = bounds::Plane::fromPointNormal(vec3{ 1.0f, 0.0f, 0.0f }, vec3{ 1.0f, 0.0f, 0.0f });
        auto d = plane.d();
        REQUIRE(fequals(d, 1.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("plane from point-normal contains the point", "[math::bounds]") {
        const vec3 point{ 10.0f, 0.0f, 0.0f };
        const vec3 normal{ 1.0f, 0.0f, 0.0f };
        auto plane = bounds::Plane::fromPointNormal(point, normal);

        // The input point should lie on the plane (signed distance ≈ 0).
        REQUIRE(fequals(plane.signedDistance(point), 0.0f));

        // A point 5 units along the normal should have signed distance +5.
        REQUIRE(fequals(plane.signedDistance(point + normal * 5.0f), 5.0f));

        // A point 5 units opposite the normal should have signed distance -5.
        REQUIRE(fequals(plane.signedDistance(point - normal * 5.0f), -5.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("plane flip", "[math::bounds]")
    {
        auto plane = bounds::Plane::fromPointNormal(vec3{ 1.0f, 0.0f, 0.0f }, vec3{ 1.0f, 0.0f, 0.0f });
        auto flipped = plane.flipped();

        REQUIRE(flipped.value == -plane.value);
    } LITL_END_TEST_CASE
        
    // -------------------------------------------------------------------------------------
    // Frustum
    // -------------------------------------------------------------------------------------

    namespace
    {
        constexpr float commonCornersHalfSize = 1.0f;

        static bounds::FrustumCorners commonCorners {
            .nearLL = vec3{-commonCornersHalfSize, -commonCornersHalfSize, 0.0f },
            .nearLR = vec3{ commonCornersHalfSize, -commonCornersHalfSize, 0.0f },
            .nearUR = vec3{ commonCornersHalfSize,  commonCornersHalfSize, 0.0f },
            .nearUL = vec3{-commonCornersHalfSize,  commonCornersHalfSize, 0.0f },
            .farLL = vec3{-commonCornersHalfSize, -commonCornersHalfSize, commonCornersHalfSize * 2.0f},
            .farLR = vec3{ commonCornersHalfSize, -commonCornersHalfSize, commonCornersHalfSize * 2.0f},
            .farUR = vec3{ commonCornersHalfSize,  commonCornersHalfSize, commonCornersHalfSize * 2.0f},
            .farUL = vec3{-commonCornersHalfSize,  commonCornersHalfSize, commonCornersHalfSize * 2.0f}
        };
    }

    LITL_TEST_CASE("frustum from corners", "[math::bounds]")
    {
        auto frustum = bounds::Frustum::fromCorners(commonCorners, {});
        auto extractedCorners = bounds::Frustum::extractCorners(frustum);

        REQUIRE(extractedCorners.nearLL == commonCorners.nearLL);
        REQUIRE(extractedCorners.nearLR == commonCorners.nearLR);
        REQUIRE(extractedCorners.nearUR == commonCorners.nearUR);
        REQUIRE(extractedCorners.nearUL == commonCorners.nearUL);
        REQUIRE(extractedCorners.farLL == commonCorners.farLL);
        REQUIRE(extractedCorners.farLR == commonCorners.farLR);
        REQUIRE(extractedCorners.farUR == commonCorners.farUR);
        REQUIRE(extractedCorners.farUL == commonCorners.farUL);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("frustum planes face inward", "[math::bounds]")
    {
        // Interior point should have positive signed distance to all planes
        auto frustum = bounds::Frustum::fromCorners(commonCorners, {});
        const vec3 interior{ 0.0f, 0.0f, commonCornersHalfSize };

        for (uint32_t i = 0; i < frustum.sideCount(); ++i)
        {
            float distance = frustum.getSide(static_cast<bounds::Frustum::Side>(i)).signedDistance(interior);
            REQUIRE(distance > 0.0f);
        }

        // Each corner should lie exactly on 3 planes (distance ~= 0) and inside the other 3
        REQUIRE(abs(frustum.getSide(bounds::Frustum::Side::Left).signedDistance(commonCorners.nearLL)) < 1e-5f);
        REQUIRE(abs(frustum.getSide(bounds::Frustum::Side::Bottom).signedDistance(commonCorners.nearLL)) < 1e-5f);
        REQUIRE(abs(frustum.getSide(bounds::Frustum::Side::Near).signedDistance(commonCorners.nearLL)) < 1e-5f);
        // and so on for each corner
    } LITL_END_TEST_CASE

#include <iostream>

    LITL_TEST_CASE("fromViewProjection produces inward-facing planes", "[math::bounds]")
    {
        // Standard perspective, reverse-Z, Vulkan-style
        const auto targetPos = vec3{ 0.0f, 0.0f, 0.0f };
        const auto view = mat4::lookAt(vec3{ 0, 0, -5 }, targetPos, vec3{ 0, 1, 0 });
        const auto proj = mat4::perspective(degreesToRadians(60.0f), 1.0f, 0.1f, 100.0f);
        const auto vp = proj * view;
        const auto frustum = bounds::Frustum::fromViewProjection(vp, {});

        const auto leftPlane = frustum.getSide(bounds::Frustum::Side::Left);
        std::cout << "Left plane normal: ("
            << leftPlane.normal().x() << ", "
            << leftPlane.normal().y() << ", "
            << leftPlane.normal().z() << ")\n";
        std::cout << "Left plane d: " << leftPlane.d() << "\n";
        std::cout << "signedDistance to origin: " << leftPlane.signedDistance(vec3{ 0,0,0 }) << "\n";
        std::cout << "signedDistance to (-10, 0, 0): " << leftPlane.signedDistance(vec3{ -10,0,0 }) << "\n";
        std::cout << "signedDistance to (+10, 0, 0): " << leftPlane.signedDistance(vec3{ +10,0,0 }) << "\n";

        // A point in front of the camera (within the frustum) should be inside all planes.
        for (uint32_t i = 0; i < frustum.sideCount(); ++i)
        {
            const float d = frustum.getSide(static_cast<bounds::Frustum::Side>(i)).signedDistance(targetPos);
            REQUIRE(d > 0.0f);
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("frustum rejects points outside each plane", "[math::bounds]")
    {
        // A point far outside each plane should not be contained.
        auto frustum = bounds::Frustum::fromCorners(commonCorners, {});

        REQUIRE(!contains(frustum, vec3{ -commonCornersHalfSize * 2, 0, commonCornersHalfSize }));  // left of Left
        REQUIRE(!contains(frustum, vec3{ commonCornersHalfSize * 2, 0, commonCornersHalfSize }));   // right of Right
        REQUIRE(!contains(frustum, vec3{ 0, -commonCornersHalfSize * 2, commonCornersHalfSize }));  // below Bottom
        REQUIRE(!contains(frustum, vec3{ 0,  commonCornersHalfSize * 2, commonCornersHalfSize }));  // above Top
        REQUIRE(!contains(frustum, vec3{ 0, 0, -commonCornersHalfSize }));                          // behind Near
        REQUIRE(!contains(frustum, vec3{ 0, 0, commonCornersHalfSize * 4 }));                       // past Far
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("every corner lies on its three planes", "[math::bounds]")
    {
        auto frustum = bounds::Frustum::fromCorners(commonCorners, {});

        struct CornerPlanes {
            vec3 corner;
            std::array<bounds::Frustum::Side, 3> onPlanes;
        };

        const std::array<CornerPlanes, 8> cornerCases = { {
            {commonCorners.nearLL, {bounds::Frustum::Near, bounds::Frustum::Bottom, bounds::Frustum::Left}},
            {commonCorners.nearLR, {bounds::Frustum::Near, bounds::Frustum::Bottom, bounds::Frustum::Right}},
            {commonCorners.nearUR, {bounds::Frustum::Near, bounds::Frustum::Top,    bounds::Frustum::Right}},
            {commonCorners.nearUL, {bounds::Frustum::Near, bounds::Frustum::Top,    bounds::Frustum::Left}},
            {commonCorners.farLL,  {bounds::Frustum::Far,  bounds::Frustum::Bottom, bounds::Frustum::Left}},
            {commonCorners.farLR,  {bounds::Frustum::Far,  bounds::Frustum::Bottom, bounds::Frustum::Right}},
            {commonCorners.farUR,  {bounds::Frustum::Far,  bounds::Frustum::Top,    bounds::Frustum::Right}},
            {commonCorners.farUL,  {bounds::Frustum::Far,  bounds::Frustum::Top,    bounds::Frustum::Left}},
        } };

        for (const auto& cornerCase : cornerCases)
        {
            for (auto const& plane : cornerCase.onPlanes)
            {
                const float d = frustum.getSide(plane).signedDistance(cornerCase.corner);
                REQUIRE(isZero(d));
            }
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("infinite far frustum has 5 planes", "[math::bounds]") 
    {
        const auto view = mat4::lookAt(vec3{ 0, 0, -5 }, vec3{ 0, 0, 0 }, vec3{ 0, 1, 0 });
        const auto proj = mat4::perspective(degreesToRadians(60.0f), 1.0f, 0.1f, 100.0f);
        const auto vp = proj * view;
        const auto frustum = bounds::Frustum::fromViewProjection(vp, { .useInfiniteFar = true });

        REQUIRE(frustum.sideCount() == 5);
        REQUIRE(frustum.isInfiniteZ());
        REQUIRE(frustum.getAllSides().size() == 5);

        // A point arbitrarily far along the forward direction should still be contained since there's no far plane to reject it.
        const vec3 veryFar{ 0, 0, 1e6f };

        REQUIRE(contains(frustum, veryFar) == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("reverseZ flag produces same geometric frustum", "[math::bounds]") 
    {
        // Build two matrices that represent the same frustum, one standard-Z one reverse-Z.
        const auto view = mat4::lookAt(vec3{ 0, 0, -5 }, vec3{ 0, 0, 0 }, vec3{ 0, 1, 0 });
        const auto proj = mat4::perspective(degreesToRadians(60.0f), 1.0f, 0.1f, 100.0f);
        const auto vp = proj * view;

        const auto f1 = bounds::Frustum::fromViewProjection(vp, { .reverseZ = false });
        const auto f2 = bounds::Frustum::fromViewProjection(vp, { .reverseZ = true });

        // Side planes should be identical
        for (auto side : { bounds::Frustum::Left, bounds::Frustum::Right, bounds::Frustum::Top,  bounds::Frustum::Bottom }) 
        {
            REQUIRE(f1.getSide(side).value == f2.getSide(side).value);
        }

        // Near/Far should be swapped
        REQUIRE(f1.getSide(bounds::Frustum::Near).value == f2.getSide(bounds::Frustum::Far).value);
        REQUIRE(f1.getSide(bounds::Frustum::Far).value == f2.getSide(bounds::Frustum::Near).value);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("normalization doesn't change contains results", "[math::bounds]") 
    {
        auto f_norm = bounds::Frustum::fromCorners(commonCorners, { .normalize = true });
        auto f_unnorm = bounds::Frustum::fromCorners(commonCorners, { .normalize = false });

        const std::array<vec3, 6> testPoints = {
            vec3{0, 0, commonCornersHalfSize},                              // inside
            vec3{commonCornersHalfSize * 2, 0, commonCornersHalfSize},      // outside right
            vec3{0, 0, -commonCornersHalfSize},                             // outside near
            vec3{commonCornersHalfSize, 0, commonCornersHalfSize}           // on boundary
        };

        for (const auto& p : testPoints) 
        {
            REQUIRE(contains(f_norm, p) == contains(f_unnorm, p));
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("points exactly on the frustum boundary are contained", "[math::bounds]") 
    {
        auto frustum = bounds::Frustum::fromCorners(commonCorners, {});

        // The 8 corners are on the boundary — they should all be contained (inclusive check).
        REQUIRE(contains(frustum, commonCorners.nearLL));
        REQUIRE(contains(frustum, commonCorners.nearLR));
        REQUIRE(contains(frustum, commonCorners.nearUR));
        REQUIRE(contains(frustum, commonCorners.nearUL));
        REQUIRE(contains(frustum, commonCorners.farLL));
        REQUIRE(contains(frustum, commonCorners.farLR));
        REQUIRE(contains(frustum, commonCorners.farUR));
        REQUIRE(contains(frustum, commonCorners.farUL));

        // Midpoints of edges
        REQUIRE(contains(frustum, (commonCorners.nearLL + commonCorners.farLL) * 0.5f));
        REQUIRE(contains(frustum, (commonCorners.nearLL + commonCorners.nearUR) * 0.5f));

        // The geometric center
        REQUIRE(contains(frustum, (commonCorners.nearLL + commonCorners.farUR) * 0.5f));
    } LITL_END_TEST_CASE
}