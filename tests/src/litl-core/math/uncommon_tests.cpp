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
}