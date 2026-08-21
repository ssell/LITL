#include "tests.hpp"
#include "litl-core/math/uncommon.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("orthonormal basis right-handed", "[math::uncommon]")
    {
        vec3 normal = vec3::up();
        vec3 forward, right;

        orthonormalBasisRH(normal, right, forward);

        REQUIRE(normal == vec3::up());
        REQUIRE(right == vec3::right());
        REQUIRE(forward == -vec3::forward());
    } LITL_END_TEST_CASE
        
    LITL_TEST_CASE("orthonormal basis left-handed", "[math::uncommon]")
    {
        vec3 normal = vec3::up();
        vec3 forward, right;

        orthonormalBasis(normal, right, forward);

        REQUIRE(normal == vec3::up());
        REQUIRE(right == vec3::right());
        REQUIRE(forward == vec3::forward());
    } LITL_END_TEST_CASE
}