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

    LITL_TEST_CASE("aabb p-vertex", "[math::bounds]")
    {
        const bounds::AABB aabb{ .min{ -1.0f, -1.0f, -1.0f}, .max{ 1.0f, 1.0f, 1.0f} };

        REQUIRE(aabb.pVertex(vec3{ 1.0f, 0.0f, 0.0f }) == vec3{ 1.0f, 1.0f, 1.0f });
        REQUIRE(aabb.pVertex(vec3{ -1.0f, 0.0f, 0.0f }) == vec3{ -1.0f, 1.0f, 1.0f });
        REQUIRE(aabb.pVertex(vec3{ 0.0f, 1.0f, 0.0f }) == vec3{ 1.0f, 1.0f, 1.0f });
        REQUIRE(aabb.pVertex(vec3{ 0.0f, -1.0f, 0.0f }) == vec3{ 1.0f, -1.0f, 1.0f });
        REQUIRE(aabb.pVertex(vec3{ 0.0f, 0.0f, 1.0f }) == vec3{ 1.0f, 1.0f, 1.0f });
        REQUIRE(aabb.pVertex(vec3{ 0.0f, 0.0f, -1.0f }) == vec3{ 1.0f, 1.0f, -1.0f });
        REQUIRE(aabb.pVertex(vec3{ -1.0f, -1.0f, 0.0f }) == vec3{ -1.0f, -1.0f, 1.0f });
        REQUIRE(aabb.pVertex(vec3{ 0.0f, -1.0f, -1.0f }) == vec3{ 1.0f, -1.0f, -1.0f });
        REQUIRE(aabb.pVertex(vec3{ -1.0f, 0.0f, -1.0f }) == vec3{ -1.0f, 1.0f, -1.0f });
        REQUIRE(aabb.pVertex(vec3{ -1.0f, -1.0f, -1.0f }) == vec3{ -1.0f, -1.0f, -1.0f });
        REQUIRE(aabb.pVertex(vec3{ 1.0f, 1.0f, 1.0f }) == vec3{ 1.0f, 1.0f, 1.0f });
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("aabb n-vertex", "[math::bounds]")
    {
        const bounds::AABB aabb{ .min{ -1.0f, -1.0f, -1.0f}, .max{ 1.0f, 1.0f, 1.0f} };

        REQUIRE(aabb.nVertex(vec3{ 1.0f, 0.0f, 0.0f }) == vec3{ -1.0f, -1.0f, -1.0f });
        REQUIRE(aabb.nVertex(vec3{ -1.0f, 0.0f, 0.0f }) == vec3{ 1.0f, -1.0f, -1.0f });
        REQUIRE(aabb.nVertex(vec3{ 0.0f, 1.0f, 0.0f }) == vec3{ -1.0f, -1.0f, -1.0f });
        REQUIRE(aabb.nVertex(vec3{ 0.0f, -1.0f, 0.0f }) == vec3{ -1.0f, 1.0f, -1.0f });
        REQUIRE(aabb.nVertex(vec3{ 0.0f, 0.0f, 1.0f }) == vec3{ -1.0f, -1.0f, -1.0f });
        REQUIRE(aabb.nVertex(vec3{ 0.0f, 0.0f, -1.0f }) == vec3{ -1.0f, -1.0f, 1.0f });
        REQUIRE(aabb.nVertex(vec3{ -1.0f, -1.0f, 0.0f }) == vec3{ 1.0f, 1.0f, -1.0f });
        REQUIRE(aabb.nVertex(vec3{ 0.0f, -1.0f, -1.0f }) == vec3{ -1.0f, 1.0f, 1.0f });
        REQUIRE(aabb.nVertex(vec3{ -1.0f, 0.0f, -1.0f }) == vec3{ 1.0f, -1.0f, 1.0f });
        REQUIRE(aabb.nVertex(vec3{ -1.0f, -1.0f, -1.0f }) == vec3{ 1.0f, 1.0f, 1.0f });
        REQUIRE(aabb.nVertex(vec3{ 1.0f, 1.0f, 1.0f }) == vec3{ -1.0f, -1.0f, -1.0f });
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

    LITL_TEST_CASE("plane contains sphere", "[math::bounds]")
    {
        auto plane = bounds::Plane::fromPointNormal(vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.0f, 1.0f, 0.0f });
        auto sphereInside = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 1.0f, 0.0f }, 0.5f);
        auto sphereStraddle = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 0.25f, 0.0f }, 0.5f);
        auto sphereOutside = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, -1.0f, 0.0f }, 0.5f);

        REQUIRE(bounds::contains(plane, sphereInside) == true);
        REQUIRE(bounds::contains(plane, sphereStraddle) == false);
        REQUIRE(bounds::contains(plane, sphereOutside) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("plane intersects sphere", "[math::bounds]")
    {
        auto plane = bounds::Plane::fromPointNormal(vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.0f, 1.0f, 0.0f });
        auto sphereInside = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 1.0f, 0.0f }, 0.5f);
        auto sphereStraddle = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 0.25f, 0.0f }, 0.5f);
        auto sphereOutside = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, -1.0f, 0.0f }, 0.5f);

        REQUIRE(bounds::intersects(plane, sphereInside) == false);
        REQUIRE(bounds::intersects(plane, sphereStraddle) == true);
        REQUIRE(bounds::intersects(plane, sphereOutside) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("sphere outside plane", "[math::bounds]")
    {
        auto plane = bounds::Plane::fromPointNormal(vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.0f, 1.0f, 0.0f });
        auto sphereInside = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 1.0f, 0.0f }, 0.5f);
        auto sphereStraddle = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 0.25f, 0.0f }, 0.5f);
        auto sphereOutside = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, -1.0f, 0.0f }, 0.5f);

        REQUIRE(bounds::isOutside(plane, sphereInside) == false);
        REQUIRE(bounds::isOutside(plane, sphereStraddle) == false);
        REQUIRE(bounds::isOutside(plane, sphereOutside) == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("plane contains aabb", "[math::bounds]")
    {
        auto plane = bounds::Plane::fromPointNormal(vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.0f, 1.0f, 0.0f });
        auto aabbInside = bounds::AABB::fromMinMax(vec3{ 1.0f, 1.0f, 1.0f }, vec3{ 2.0f, 2.0f, 2.0f });
        auto aabbStraddle = bounds::AABB::fromMinMax(vec3{ -1.0f, -1.0f, -1.0f }, vec3{ 1.0f, 1.0f, 1.0f });
        auto aabbOutside = bounds::AABB::fromMinMax(vec3{ -2.0f, -2.0f, -2.0f }, vec3{ -1.0f, -1.0f, -1.0f });

        REQUIRE(bounds::contains(plane, aabbInside) == true);
        REQUIRE(bounds::contains(plane, aabbStraddle) == false);
        REQUIRE(bounds::contains(plane, aabbOutside) == false);
    } LITL_END_TEST_CASE
        

    LITL_TEST_CASE("plane intersects aabb", "[math::bounds]")
    {
        auto plane = bounds::Plane::fromPointNormal(vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.0f, 1.0f, 0.0f });
        auto aabbInside = bounds::AABB::fromMinMax(vec3{ 1.0f, 1.0f, 1.0f }, vec3{ 2.0f, 2.0f, 2.0f });
        auto aabbStraddle = bounds::AABB::fromMinMax(vec3{ -1.0f, -1.0f, -1.0f }, vec3{ 1.0f, 1.0f, 1.0f });
        auto aabbOutside = bounds::AABB::fromMinMax(vec3{ -2.0f, -2.0f, -2.0f }, vec3{ -1.0f, -1.0f, -1.0f });

        REQUIRE(bounds::intersects(plane, aabbInside) == false);
        REQUIRE(bounds::intersects(plane, aabbStraddle) == true);
        REQUIRE(bounds::intersects(plane, aabbOutside) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("aabb outside plane", "[math::bounds]")
    {
        auto plane = bounds::Plane::fromPointNormal(vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.0f, 1.0f, 0.0f });
        auto aabbInside = bounds::AABB::fromMinMax(vec3{ 1.0f, 1.0f, 1.0f }, vec3{ 2.0f, 2.0f, 2.0f });
        auto aabbStraddle = bounds::AABB::fromMinMax(vec3{ -1.0f, -1.0f, -1.0f }, vec3{ 1.0f, 1.0f, 1.0f });
        auto aabbOutside = bounds::AABB::fromMinMax(vec3{ -2.0f, -2.0f, -2.0f }, vec3{ -1.0f, -1.0f, -1.0f });

        REQUIRE(bounds::isOutside(plane, aabbInside) == false);
        REQUIRE(bounds::isOutside(plane, aabbStraddle) == false);
        REQUIRE(bounds::isOutside(plane, aabbOutside) == true);
    } LITL_END_TEST_CASE
        
    // -------------------------------------------------------------------------------------
    // Frustum
    // -------------------------------------------------------------------------------------

    namespace
    {
        /// <summary>
        /// Constructs an unit-cube frustum that ranges from:
        ///     x: [-1, 1]
        ///     y: [-1, 1]
        ///     z: [-1, 1]
        /// </summary>
        static bounds::FrustumCorners unitCubeCorners {
            .nearLL = vec3{-1.0f, -1.0f, -1.0f },
            .nearLR = vec3{ 1.0f, -1.0f, -1.0f },
            .nearUR = vec3{ 1.0f,  1.0f, -1.0f },
            .nearUL = vec3{-1.0f,  1.0f, -1.0f },
            .farLL  = vec3{-1.0f, -1.0f, 1.0f},
            .farLR  = vec3{ 1.0f, -1.0f, 1.0f},
            .farUR  = vec3{ 1.0f,  1.0f, 1.0f},
            .farUL  = vec3{-1.0f,  1.0f, 1.0f}
        };
    }

    LITL_TEST_CASE("frustum from corners", "[math::bounds]")
    {
        auto frustum = bounds::Frustum::fromCorners(unitCubeCorners, {});
        auto extractedCorners = bounds::Frustum::extractCorners(frustum);

        REQUIRE(extractedCorners.nearLL == unitCubeCorners.nearLL);
        REQUIRE(extractedCorners.nearLR == unitCubeCorners.nearLR);
        REQUIRE(extractedCorners.nearUR == unitCubeCorners.nearUR);
        REQUIRE(extractedCorners.nearUL == unitCubeCorners.nearUL);
        REQUIRE(extractedCorners.farLL == unitCubeCorners.farLL);
        REQUIRE(extractedCorners.farLR == unitCubeCorners.farLR);
        REQUIRE(extractedCorners.farUR == unitCubeCorners.farUR);
        REQUIRE(extractedCorners.farUL == unitCubeCorners.farUL);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("frustum planes face inward", "[math::bounds]")
    {
        // Interior point should have positive signed distance to all planes
        auto frustum = bounds::Frustum::fromCorners(unitCubeCorners, {});
        const vec3 interior{ 0.0f, 0.0f, 0.5f };

        for (uint32_t i = 0; i < frustum.sideCount(); ++i)
        {
            float distance = frustum.getSide(static_cast<bounds::Frustum::Side>(i)).signedDistance(interior);
            REQUIRE(distance > 0.0f);
        }

        const auto left = frustum.getSide(bounds::Frustum::Side::Left);
        const auto right = frustum.getSide(bounds::Frustum::Side::Right);
        const auto top = frustum.getSide(bounds::Frustum::Side::Top);
        const auto bottom = frustum.getSide(bounds::Frustum::Side::Bottom);
        const auto near = frustum.getSide(bounds::Frustum::Side::Near);
        const auto far = frustum.getSide(bounds::Frustum::Side::Far);

        // Each corner should lie exactly on 3 planes (distance ~= 0) and inside the other 3
        REQUIRE(isZero(left.signedDistance(unitCubeCorners.nearLL)));
        REQUIRE(isZero(bottom.signedDistance(unitCubeCorners.nearLL)));
        REQUIRE(isZero(near.signedDistance(unitCubeCorners.nearLL)));

        REQUIRE(isZero(right.signedDistance(unitCubeCorners.nearLR)));
        REQUIRE(isZero(bottom.signedDistance(unitCubeCorners.nearLR)));
        REQUIRE(isZero(near.signedDistance(unitCubeCorners.nearLR)));

        REQUIRE(isZero(right.signedDistance(unitCubeCorners.nearUR)));
        REQUIRE(isZero(top.signedDistance(unitCubeCorners.nearUR)));
        REQUIRE(isZero(near.signedDistance(unitCubeCorners.nearUR)));

        REQUIRE(isZero(left.signedDistance(unitCubeCorners.nearUL)));
        REQUIRE(isZero(top.signedDistance(unitCubeCorners.nearUL)));
        REQUIRE(isZero(near.signedDistance(unitCubeCorners.nearUL)));

        REQUIRE(isZero(left.signedDistance(unitCubeCorners.farLL)));
        REQUIRE(isZero(bottom.signedDistance(unitCubeCorners.farLL)));
        REQUIRE(isZero(far.signedDistance(unitCubeCorners.farLL)));

        REQUIRE(isZero(right.signedDistance(unitCubeCorners.farLR)));
        REQUIRE(isZero(bottom.signedDistance(unitCubeCorners.farLR)));
        REQUIRE(isZero(far.signedDistance(unitCubeCorners.farLR)));

        REQUIRE(isZero(right.signedDistance(unitCubeCorners.farUR)));
        REQUIRE(isZero(top.signedDistance(unitCubeCorners.farUR)));
        REQUIRE(isZero(far.signedDistance(unitCubeCorners.farUR)));

        REQUIRE(isZero(left.signedDistance(unitCubeCorners.farUL)));
        REQUIRE(isZero(top.signedDistance(unitCubeCorners.farUL)));
        REQUIRE(isZero(far.signedDistance(unitCubeCorners.farUL)));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("fromViewProjection produces inward-facing planes", "[math::bounds]")
    {
        // Standard perspective, reverse-Z, Vulkan-style
        const auto targetPos = vec3{ 0.0f, 0.0f, 0.0f };
        const auto view = mat4::lookAt(vec3{ 0, 0, -5 }, targetPos, vec3{ 0, 1, 0 });
        const auto proj = mat4::perspective(degreesToRadians(60.0f), 1.0f, 0.1f, 100.0f);
        const auto vp = proj * view;
        const auto frustum = bounds::Frustum::fromViewProjection(vp, {});

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
        auto frustum = bounds::Frustum::fromCorners(unitCubeCorners, {});

        REQUIRE(!contains(frustum, vec3{ -1.0f * 2, 0, 1.0f }));    // left of Left
        REQUIRE(!contains(frustum, vec3{ 1.0f * 2, 0, 1.0f }));     // right of Right
        REQUIRE(!contains(frustum, vec3{ 0, -1.0f * 2, 1.0f }));    // below Bottom
        REQUIRE(!contains(frustum, vec3{ 0,  1.0f * 2, 1.0f }));    // above Top
        REQUIRE(!contains(frustum, vec3{ 0, 0, -1.1f }));           // behind Near
        REQUIRE(!contains(frustum, vec3{ 0, 0, 1.1f }));            // past Far
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("every corner lies on its three planes", "[math::bounds]")
    {
        auto frustum = bounds::Frustum::fromCorners(unitCubeCorners, {});

        struct CornerPlanes {
            vec3 corner;
            std::array<bounds::Frustum::Side, 3> onPlanes;
        };

        const std::array<CornerPlanes, 8> cornerCases = { {
            {unitCubeCorners.nearLL, {bounds::Frustum::Near, bounds::Frustum::Bottom, bounds::Frustum::Left}},
            {unitCubeCorners.nearLR, {bounds::Frustum::Near, bounds::Frustum::Bottom, bounds::Frustum::Right}},
            {unitCubeCorners.nearUR, {bounds::Frustum::Near, bounds::Frustum::Top,    bounds::Frustum::Right}},
            {unitCubeCorners.nearUL, {bounds::Frustum::Near, bounds::Frustum::Top,    bounds::Frustum::Left}},
            {unitCubeCorners.farLL,  {bounds::Frustum::Far,  bounds::Frustum::Bottom, bounds::Frustum::Left}},
            {unitCubeCorners.farLR,  {bounds::Frustum::Far,  bounds::Frustum::Bottom, bounds::Frustum::Right}},
            {unitCubeCorners.farUR,  {bounds::Frustum::Far,  bounds::Frustum::Top,    bounds::Frustum::Right}},
            {unitCubeCorners.farUL,  {bounds::Frustum::Far,  bounds::Frustum::Top,    bounds::Frustum::Left}},
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
        auto f_norm = bounds::Frustum::fromCorners(unitCubeCorners, { .normalize = true });
        auto f_unnorm = bounds::Frustum::fromCorners(unitCubeCorners, { .normalize = false });

        const std::array<vec3, 6> testPoints = {
            vec3{0, 0, 1.0f},           // inside
            vec3{1.0f * 2, 0, 1.0f},    // outside right
            vec3{0, 0, -1.0f},          // outside near
            vec3{1.0f, 0, 1.0f}         // on boundary
        };

        for (const auto& p : testPoints) 
        {
            REQUIRE(contains(f_norm, p) == contains(f_unnorm, p));
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("points exactly on the frustum boundary are contained", "[math::bounds]") 
    {
        auto frustum = bounds::Frustum::fromCorners(unitCubeCorners, {});

        // The 8 corners are on the boundary — they should all be contained (inclusive check).
        REQUIRE(contains(frustum, unitCubeCorners.nearLL));
        REQUIRE(contains(frustum, unitCubeCorners.nearLR));
        REQUIRE(contains(frustum, unitCubeCorners.nearUR));
        REQUIRE(contains(frustum, unitCubeCorners.nearUL));
        REQUIRE(contains(frustum, unitCubeCorners.farLL));
        REQUIRE(contains(frustum, unitCubeCorners.farLR));
        REQUIRE(contains(frustum, unitCubeCorners.farUR));
        REQUIRE(contains(frustum, unitCubeCorners.farUL));

        // Midpoints of edges
        REQUIRE(contains(frustum, (unitCubeCorners.nearLL + unitCubeCorners.farLL) * 0.5f));
        REQUIRE(contains(frustum, (unitCubeCorners.nearLL + unitCubeCorners.nearUR) * 0.5f));

        // The geometric center
        REQUIRE(contains(frustum, (unitCubeCorners.nearLL + unitCubeCorners.farUR) * 0.5f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("frustum contains sphere", "[math::bounds]")
    {
        auto frustum = bounds::Frustum::fromCorners(unitCubeCorners, {});
        auto sphereInside = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 0.0f, 0.0f }, 0.5f);
        auto sphereStraddle = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 1.0f, 0.0f }, 0.5f);     // straddle top plane
        auto sphereOutside = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 2.0f, 0.0f }, 0.5f);      // outside top plane

        REQUIRE(contains(frustum, sphereInside) == true);
        REQUIRE(contains(frustum, sphereStraddle) == false);
        REQUIRE(contains(frustum, sphereOutside) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("frustum intersects sphere", "[math::bounds]")
    {
        auto frustum = bounds::Frustum::fromCorners(unitCubeCorners, {});
        auto sphereInside = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 0.0f, 0.0f }, 0.5f);
        auto sphereStraddle = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 1.0f, 0.0f }, 0.5f);     // straddle top plane
        auto sphereOutside = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 2.0f, 0.0f }, 0.5f);      // outside top plane

        REQUIRE(intersects(frustum, sphereInside) == true);
        REQUIRE(intersects(frustum, sphereStraddle) == true);
        REQUIRE(intersects(frustum, sphereOutside) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("frustum classify sphere", "[math::bounds]")
    {
        auto frustum = bounds::Frustum::fromCorners(unitCubeCorners, {});
        auto sphereInside = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 0.0f, 0.0f }, 0.5f);
        auto sphereStraddle = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 1.0f, 0.0f }, 0.5f);         // straddle top plane
        auto sphereOutside = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 2.0f, 0.0f }, 0.5f);          // outside top plane
        auto sphereOutsideAll = bounds::Sphere::fromCenterRadius(vec3{ -100.0f, 0.0f, 0.0f }, 10.0f);   // completely outside the frustum
        auto sphereCover = bounds::Sphere::fromCenterRadius(vec3{ -100.0f, 0.0f, 0.0f }, 1000.0f);      // cover frustum (straddle all)

        auto sphereInsideClassification = bounds::classify(frustum, sphereInside);
        auto sphereStraddleClassification = bounds::classify(frustum, sphereStraddle);
        auto sphereOutsideClassification = bounds::classify(frustum, sphereOutside);
        auto sphereOutsideAllClassification = bounds::classify(frustum, sphereOutsideAll);
        auto sphereCoverClassification = bounds::classify(frustum, sphereCover);

        REQUIRE(sphereInsideClassification.type() == bounds::FrustumClassification::Inside);
        REQUIRE(sphereInsideClassification.outsideMask == 0b000000);
        REQUIRE(sphereInsideClassification.straddleMask == 0b000000);

        REQUIRE(sphereStraddleClassification.type() == bounds::FrustumClassification::Intersects);
        REQUIRE(sphereStraddleClassification.outsideMask == 0b000000);
        REQUIRE(sphereStraddleClassification.straddleMask == 0b001000);   // Frustum::Side::Top == 3 (fourth, 0-indexed)

        REQUIRE(sphereOutsideClassification.type() == bounds::FrustumClassification::Outside);
        REQUIRE(sphereOutsideClassification.outsideMask == 0b001000);
        REQUIRE(sphereOutsideClassification.straddleMask == 0b000000);

        REQUIRE(sphereOutsideAllClassification.type() == bounds::FrustumClassification::Outside);
        REQUIRE(sphereOutsideAllClassification.outsideMask == 0b000001);    // Classification early exits when it detects the sphere is fully outside at least one plane.
        REQUIRE(sphereOutsideAllClassification.straddleMask == 0b000000);

        REQUIRE(sphereCoverClassification.type() == bounds::FrustumClassification::Intersects);
        REQUIRE(sphereCoverClassification.outsideMask == 0b000000);
        REQUIRE(sphereCoverClassification.straddleMask == 0b111111);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("frustum classify sphere active mask", "[math::bounds]")
    {
        auto frustum = bounds::Frustum::fromCorners(unitCubeCorners, {});
        auto sphereOutside = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 2.0f, 0.0f }, 0.5f);      // outside top plane

        // bit set to 0 = inside, bit set to 1 = straddle OR outside
        const auto allInsideBitMask = 0b000000;

        // although this sphere is fully outside of the frustum, it should be classified as inside 
        // as the passed in active mask is signaling it (or well, its parent) as inside.
        // this is naturally an invalid mask for a well-formed parent/child.
        auto classification = bounds::classify(frustum, sphereOutside, allInsideBitMask);

        REQUIRE(classification.type() == bounds::FrustumClassification::Inside);
        REQUIRE(classification.outsideMask == 0b000000);
        REQUIRE(classification.straddleMask == 0b000000);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("frustum contains aabb", "[math::bounds]")
    {
        auto frustum = bounds::Frustum::fromCorners(unitCubeCorners, {});
        auto aabbInside = bounds::AABB::fromMinMax(vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.5f, 0.5f, 0.5f });
        auto aabbStraddle = bounds::AABB::fromMinMax(vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.5f, 1.5f, 0.5f });           // straddle top plane
        auto aabbOutside = bounds::AABB::fromMinMax(vec3{ 0.5f, 1.5f, 0.5f }, vec3{ 0.5f, 2.0f, 0.5f });            // outside top plane

        REQUIRE(contains(frustum, aabbInside) == true);
        REQUIRE(contains(frustum, aabbStraddle) == false);
        REQUIRE(contains(frustum, aabbOutside) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("frustum classify aabb", "[math::bounds]")
    {
        auto frustum = bounds::Frustum::fromCorners(unitCubeCorners, {});
        auto aabbInside = bounds::AABB::fromMinMax(vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.5f, 0.5f, 0.5f });
        auto aabbStraddle = bounds::AABB::fromMinMax(vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.5f, 1.5f, 0.5f });                   // straddle top plane
        auto aabbOutside = bounds::AABB::fromMinMax(vec3{ 0.5f, 1.5f, 0.5f }, vec3{ 0.5f, 2.0f, 0.5f });                    // outside top plane
        auto aabbOutsideAll = bounds::AABB::fromMinMax(vec3{ -100.0f, -100.0f, -100.0f }, vec3{ -90.0f, -90.0f, -90.0f });  // completely outside the frustum
        auto aabbCover = bounds::AABB::fromMinMax(vec3{ -10.0f, -10.0f, -10.0f }, vec3{ 10.0f, 10.0f, 10.0f });             // cover frustum (straddle all)

        auto aabbInsideClassification = bounds::classify(frustum, aabbInside);
        auto aabbStraddleClassification = bounds::classify(frustum, aabbStraddle);
        auto aabbOutsideClassification = bounds::classify(frustum, aabbOutside);
        auto aabbOutsideAllClassification = bounds::classify(frustum, aabbOutsideAll);
        auto aabbCoverClassification = bounds::classify(frustum, aabbCover);

        REQUIRE(aabbInsideClassification.type() == bounds::FrustumClassification::Inside);
        REQUIRE(aabbInsideClassification.outsideMask == 0b000000);
        REQUIRE(aabbInsideClassification.straddleMask == 0b000000);

        REQUIRE(aabbStraddleClassification.type() == bounds::FrustumClassification::Intersects);
        REQUIRE(aabbStraddleClassification.outsideMask == 0b000000);
        REQUIRE(aabbStraddleClassification.straddleMask == 0b001000);   // Frustum::Side::Top == 3 (fourth, 0-indexed)

        REQUIRE(aabbOutsideClassification.type() == bounds::FrustumClassification::Outside);
        REQUIRE(aabbOutsideClassification.outsideMask == 0b001000);
        REQUIRE(aabbOutsideClassification.straddleMask == 0b000000);

        REQUIRE(aabbOutsideAllClassification.type() == bounds::FrustumClassification::Outside);
        REQUIRE(aabbOutsideAllClassification.outsideMask == 0b000001);    // Classification early exits when it detects the AABB is fully outside at least one plane.
        REQUIRE(aabbOutsideAllClassification.straddleMask == 0b000000);

        REQUIRE(aabbCoverClassification.type() == bounds::FrustumClassification::Intersects);
        REQUIRE(aabbCoverClassification.outsideMask == 0b000000);
        REQUIRE(aabbCoverClassification.straddleMask == 0b111111);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("frustum classify aabb active mask", "[math::bounds]")
    {
        auto frustum = bounds::Frustum::fromCorners(unitCubeCorners, {});
        auto aabbOutside = bounds::AABB::fromMinMax(vec3{ 0.5f, 1.5f, 0.5f }, vec3{ 0.5f, 2.0f, 0.5f });            // outside top plane

        // bit set to 0 = inside, bit set to 1 = straddle OR outside
        const auto allInsideBitMask = 0b000000;

        // although this sphere is fully outside of the frustum, it should be classified as inside 
        // as the passed in active mask is signaling it (or well, its parent) as inside.
        // this is naturally an invalid mask for a well-formed parent/child.
        auto classification = bounds::classify(frustum, aabbOutside, allInsideBitMask);

        REQUIRE(classification.type() == bounds::FrustumClassification::Inside);
        REQUIRE(classification.outsideMask == 0b000000);
        REQUIRE(classification.straddleMask == 0b000000);
    } LITL_END_TEST_CASE
}