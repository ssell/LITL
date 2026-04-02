#include "tests.hpp"
#include "litl-engine/ecs/components/position.hpp"

namespace LITL::Engine::Tests
{
    LITL_TEST_CASE("Basic", "[engine::position]")
    {
        Position pos{ 5.0f, 0.0f, 5.0f };

        REQUIRE(Math::fequals(pos.x(), 5.0f));
        REQUIRE(Math::fequals(pos.y(), 0.0f));
        REQUIRE(Math::fequals(pos.z(), 5.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Type Safety", "[engine::position]")
    {
        // A vec3 and position may internally be identical as both are 
        // empty inheritors of Vec3Wrapper, but logically they are distinct types.
        REQUIRE(type_id<Position>() != type_id<Math::Vec3>());
        REQUIRE(type_name<Position>() != type_name<Math::Vec3>());
    } END_LITL_TEST_CASE
}