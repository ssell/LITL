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
        auto planeX = bounds::Plane::fromTriangle(vec3{ 10.0f, 0.0f, 100.0f }, vec3{ 10.0f, 0.0f, 10.0f }, vec3{ 10.0f, 10.0f, 10.0f });
        auto planeXNeg = bounds::Plane::fromTriangle(vec3{ 10.0f, 0.0f, 10.0f }, vec3{ 10.0f, 0.0f, 100.0f }, vec3{ 10.0f, 10.0f, 100.0f });

        REQUIRE(planeX.normal() == vec3::right());
        REQUIRE(fequals(planeX.d(), 10.0f));

        REQUIRE(planeXNeg.normal() == -vec3::right());
        REQUIRE(fequals(planeXNeg.d(), -10.0f));
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
        auto plane = bounds::Plane::fromPointNormal(vec3{ 0.0f, 0.0f, 0.0f }, vec3::up());

        REQUIRE(bounds::contains(plane, vec3{ 0.0f, 0.0f, 0.0f }) == true);
        REQUIRE(bounds::contains(plane, vec3{ 0.0f, 0.01f, 0.0f }) == false);
        REQUIRE(bounds::contains(plane, vec3{ 0.0f, -0.01f, 0.0f }) == true);

        REQUIRE(bounds::contains(plane, vec3{ 100.0f, 0.0f, -100.0f }) == true);
        REQUIRE(bounds::contains(plane, vec3{ 100.0f, 0.01f, -100.0f }) == false);
        REQUIRE(bounds::contains(plane, vec3{ 100.0f, -0.01f, -100.0f }) == true);
    } LITL_END_TEST_CASE
}