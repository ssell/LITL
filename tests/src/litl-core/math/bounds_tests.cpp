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

        auto planeX = bounds::Plane::fromTriangleCCW(a, b, c);      // + = counter-clockwise
        auto planeXNeg = bounds::Plane::fromTriangleCW(a, b, c);    // - = clockwise

        REQUIRE(planeX.normal() == vec3::forward());
        REQUIRE(fequals(planeX.d(), 0.0f));

        REQUIRE(planeXNeg.normal() == -vec3::forward());
        REQUIRE(fequals(planeXNeg.d(), 0.0f));
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
        auto plane = bounds::Plane::fromPointNormal(vec3{ 10.0f, 0.0f, 0.0f }, vec3{ 1.0f, 0.0f, 0.0f });
        auto d = plane.d();
        REQUIRE(fequals(d, 10.0f));
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
        
    // -------------------------------------------------------------------------------------
    // Frustum
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("frustum from corners", "[math::bounds]")
    {
        constexpr float halfSize = 10.0f;

        bounds::FrustumCorners corners{
            .nearLL = vec3{-halfSize, -halfSize, 0.0f },
            .nearLR = vec3{ halfSize, -halfSize, 0.0f },
            .nearUR = vec3{ halfSize,  halfSize, 0.0f },
            .nearUL = vec3{-halfSize,  halfSize, 0.0f },
            .farLL  = vec3{-halfSize, -halfSize, halfSize * 2.0f},
            .farLR  = vec3{ halfSize, -halfSize, halfSize * 2.0f},
            .farUR  = vec3{ halfSize,  halfSize, halfSize * 2.0f},
            .farUL  = vec3{-halfSize,  halfSize, halfSize * 2.0f}
        };

        auto frustum = bounds::Frustum::fromCorners(corners, {});
        auto extractedCorners = bounds::Frustum::extractCorners(frustum);

        REQUIRE(extractedCorners.nearLL == corners.nearLL);
        REQUIRE(extractedCorners.nearLR == corners.nearLR);
        REQUIRE(extractedCorners.nearUR == corners.nearUR);
        REQUIRE(extractedCorners.nearUL == corners.nearUL);
        REQUIRE(extractedCorners.farLL == corners.farLL);
        REQUIRE(extractedCorners.farLR == corners.farLR);
        REQUIRE(extractedCorners.farUR == corners.farUR);
        REQUIRE(extractedCorners.farUL == corners.farUL);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("frustum planes face inward", "[math::bounds]") 
    {
        constexpr float halfSize = 10.0f;

        bounds::FrustumCorners corners{
            .nearLL = vec3{-halfSize, -halfSize, 0.0f },
            .nearLR = vec3{ halfSize, -halfSize, 0.0f },
            .nearUR = vec3{ halfSize,  halfSize, 0.0f },
            .nearUL = vec3{-halfSize,  halfSize, 0.0f },
            .farLL = vec3{-halfSize, -halfSize, halfSize * 2.0f},
            .farLR = vec3{ halfSize, -halfSize, halfSize * 2.0f},
            .farUR = vec3{ halfSize,  halfSize, halfSize * 2.0f},
            .farUL = vec3{-halfSize,  halfSize, halfSize * 2.0f}
        };

        auto frustum = bounds::Frustum::fromCorners(corners, {});

        // Interior point should have positive signed distance to all planes
        const vec3 interior{ 0.0f, 0.0f, halfSize };

        for (uint32_t i = 0; i < frustum.sideCount(); ++i) 
        {
            float distance = frustum.getSide(static_cast<bounds::Frustum::Side>(i)).signedDistance(interior);
            REQUIRE(distance > 0.0f);
        }

        // Each corner should lie exactly on 3 planes (distance ~= 0) and inside the other 3
        REQUIRE(abs(frustum.getSide(bounds::Frustum::Side::Left).signedDistance(corners.nearLL)) < 1e-5f);
        REQUIRE(abs(frustum.getSide(bounds::Frustum::Side::Bottom).signedDistance(corners.nearLL)) < 1e-5f);
        REQUIRE(abs(frustum.getSide(bounds::Frustum::Side::Near).signedDistance(corners.nearLL)) < 1e-5f);
        // and so on for each corner
    } LITL_END_TEST_CASE
}