#include <array>

#include "tests.hpp"
#include "litl-core/math/uncommon.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("orthonormalBasis", "[math::uncommon]")
    {
        vec3 forward, right;

        orthonormalBasis(vec3::up(), right, forward);

        REQUIRE(right == vec3::right());
        REQUIRE(forward == vec3::forward());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("orthonormalBasisRH", "[math::uncommon]")
    {
        vec3 forward, right;

        orthonormalBasisRH(vec3::up(), right, forward);

        REQUIRE(right == vec3::right());
        REQUIRE(forward == -vec3::forward());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("project2d", "[math::uncommon]")
    {
        const vec3 normal = vec3::up();
        const vec3 point = vec3{ 50.0f, 100.0f, 150.0f };
        const vec3 origin = vec3::zero();
        const vec2 point2d = project2d(normal, point, origin);

        REQUIRE(point2d == vec2{ 50.0f, 150.0f });
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("polygonSignedArea", "[math::uncommon]")
    {
        const std::array<vec2, 4> faceCCW{
            vec2{ 0.0f, 0.0f },
            vec2{ 5.0f, 0.0f },
            vec2{ 5.0f, 5.0f },
            vec2{ 0.0f, 5.0f }
        };

        const std::array<vec2, 4> faceCW{
            vec2{ 0.0f, 0.0f },
            vec2{ 0.0f, 5.0f },
            vec2{ 5.0f, 5.0f },
            vec2{ 5.0f, 0.0f }
        };

        const float accw = polygonSignedArea(faceCCW);
        const float acw = polygonSignedArea(faceCW);

        REQUIRE(fequals(accw, 25.0f) == true);
        REQUIRE(fequals(acw, -25.0f) == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("polygonArea 2D", "[math::uncommon]")
    {
        const std::array<vec2, 4> faceCCW{
            vec2{ 0.0f, 0.0f },
            vec2{ 5.0f, 0.0f },
            vec2{ 5.0f, 5.0f },
            vec2{ 0.0f, 5.0f }
        };

        const std::array<vec2, 4> faceCW{
            vec2{ 0.0f, 0.0f },
            vec2{ 0.0f, 5.0f },
            vec2{ 5.0f, 5.0f },
            vec2{ 5.0f, 0.0f }
        };

        const float accw = polygonArea(faceCCW);
        const float acw = polygonArea(faceCW);

        REQUIRE(fequals(accw, 25.0f) == true);
        REQUIRE(fequals(acw, 25.0f) == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("polygonArea 3D", "[math::uncommon]")
    {
        const std::array<vec3, 4> faceCCW{
            vec3{ 0.0f, 0.0f, 0.0f },
            vec3{ 5.0f, 0.0f, 0.0f },
            vec3{ 5.0f, 0.0f, 5.0f },
            vec3{ 0.0f, 0.0f, 5.0f }
        };

        const std::array<vec3, 4> faceCW{
            vec3{ 0.0f, 0.0f, 0.0f },
            vec3{ 0.0f, 0.0f, 5.0f },
            vec3{ 5.0f, 0.0f, 5.0f },
            vec3{ 5.0f, 0.0f, 0.0f }
        };

        const float accw = polygonArea(faceCCW);
        const float acw = polygonArea(faceCW);

        REQUIRE(fequals(accw, 25.0f) == true);
        REQUIRE(fequals(acw, 25.0f) == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("isFaceCW 2D", "[math::uncommon]")
    {
        const std::array<vec2, 4> faceCCW{
            vec2{ 0.0f, 0.0f },
            vec2{ 5.0f, 0.0f },
            vec2{ 5.0f, 5.0f },
            vec2{ 0.0f, 5.0f }
        };

        const std::array<vec2, 4> faceCW{
            vec2{ 0.0f, 0.0f },
            vec2{ 0.0f, 5.0f },
            vec2{ 5.0f, 5.0f },
            vec2{ 5.0f, 0.0f }
        };

        const bool ccwCheck = isFaceCW(faceCCW);
        const bool cwCheck = isFaceCW(faceCW);

        REQUIRE(ccwCheck == false);
        REQUIRE(cwCheck == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("isFaceCCW 2D", "[math::uncommon]")
    {
        const std::array<vec2, 4> faceCCW{
            vec2{ 0.0f, 0.0f },
            vec2{ 5.0f, 0.0f },
            vec2{ 5.0f, 5.0f },
            vec2{ 0.0f, 5.0f }
        };

        const std::array<vec2, 4> faceCW{
            vec2{ 0.0f, 0.0f },
            vec2{ 0.0f, 5.0f },
            vec2{ 5.0f, 5.0f },
            vec2{ 5.0f, 0.0f }
        };

        const bool ccwCheck = isFaceCCW(faceCCW);
        const bool cwCheck = isFaceCCW(faceCW);

        REQUIRE(ccwCheck == true);
        REQUIRE(cwCheck == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("isFaceCW 3D", "[math::uncommon]")
    {
        const std::array<vec3, 4> faceCW{
            vec3{ 0.0f, 0.0f, 0.0f },
            vec3{ 0.0f, 5.0f, 0.0f },
            vec3{ 5.0f, 5.0f, 0.0f },
            vec3{ 5.0f, 0.0f, 0.0f }
        };

        const std::array<vec3, 4> faceCCW{
            vec3{ 0.0f, 0.0f, 0.0f },
            vec3{ 5.0f, 0.0f, 0.0f },
            vec3{ 5.0f, 5.0f, 0.0f },
            vec3{ 0.0f, 5.0f, 0.0f }
        };

        const bool cwCheck = isFaceCW(faceCW, vec3::forward());
        const bool ccwCheck = isFaceCW(faceCCW, vec3::forward());

        REQUIRE(cwCheck == true);
        REQUIRE(ccwCheck == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("isFaceCCW 3D", "[math::uncommon]")
    {
        const std::array<vec3, 4> faceCW{
            vec3{ 0.0f, 0.0f, 0.0f },
            vec3{ 0.0f, 5.0f, 0.0f },
            vec3{ 5.0f, 5.0f, 0.0f },
            vec3{ 5.0f, 0.0f, 0.0f }
        };

        const std::array<vec3, 4> faceCCW{
            vec3{ 0.0f, 0.0f, 0.0f },
            vec3{ 5.0f, 0.0f, 0.0f },
            vec3{ 5.0f, 5.0f, 0.0f },
            vec3{ 0.0f, 5.0f, 0.0f }
        };

        const bool cwCheck = isFaceCCW(faceCW, vec3::forward());
        const bool ccwCheck = isFaceCCW(faceCCW, vec3::forward());

        REQUIRE(cwCheck == false);
        REQUIRE(ccwCheck == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("centroid 3D", "[math::uncommon]")
    {
        const vec3 a(0.0f, 0.0f, 0.0f);
        const vec3 b(10.0f, 30.0f, 0.0f);
        const vec3 c(20.0f, 0.0f, 0.0f);

        const vec3 expectedCentroid = vec3(10.0f, 10.0f, 0.0f);
        const vec3 calculatedCentroid = centroid(a, b, c);

        REQUIRE(calculatedCentroid == expectedCentroid);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("centroid 2D", "[math::uncommon]")
    {
        const vec2 a(0.0f, 0.0f);
        const vec2 b(10.0f, 30.0f);
        const vec2 c(20.0f, 0.0f);

        const vec2 expectedCentroid = vec2(10.0f, 10.0f);
        const vec2 calculatedCentroid = centroid(a, b, c);

        REQUIRE(calculatedCentroid == expectedCentroid);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("pointInTriangle 3D", "[math::uncommon]")
    {
        const vec3 a(0.0f, 0.0f, 0.0f);
        const vec3 b(10.0f, 20.0f, 0.0f);
        const vec3 c(20.0f, 0.0f, 0.0f);

        REQUIRE(pointInTriangle(a, a, b, c) == true);
        REQUIRE(pointInTriangle(b, a, b, c) == true);
        REQUIRE(pointInTriangle(c, a, b, c) == true);
        REQUIRE(pointInTriangle(midpoint(a, b), a, b, c) == true);
        REQUIRE(pointInTriangle(midpoint(b, c), a, b, c) == true);
        REQUIRE(pointInTriangle(midpoint(c, a), a, b, c) == true);
        REQUIRE(pointInTriangle(centroid(a, b, c), a, b, c) == true);
        REQUIRE(pointInTriangle(-b, a, b, c) == false);
        REQUIRE(pointInTriangle(-c, a, b, c) == false);
        REQUIRE(pointInTriangle((a + b + c), a, b, c) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("pointInTriangle 2D", "[math::uncommon]")
    {
        const vec2 a(0.0f, 0.0f);
        const vec2 b(10.0f, 20.0f);
        const vec2 c(20.0f, 0.0f);

        REQUIRE(pointInTriangle(a, a, b, c) == true);
        REQUIRE(pointInTriangle(b, a, b, c) == true);
        REQUIRE(pointInTriangle(c, a, b, c) == true);
        REQUIRE(pointInTriangle(midpoint(a, b), a, b, c) == true);
        REQUIRE(pointInTriangle(midpoint(b, c), a, b, c) == true);
        REQUIRE(pointInTriangle(midpoint(c, a), a, b, c) == true);
        REQUIRE(pointInTriangle(centroid(a, b, c), a, b, c) == true);
        REQUIRE(pointInTriangle(-b, a, b, c) == false);
        REQUIRE(pointInTriangle(-c, a, b, c) == false);
        REQUIRE(pointInTriangle((a + b + c), a, b, c) == false);
    } LITL_END_TEST_CASE
}