#include <catch2/catch_test_macros.hpp>
#include "litl-engine/ecs/world.hpp"

struct Foo
{
    uint32_t value = 0;
};

struct Bar
{
    uint32_t value = 0;
};

TEST_CASE("Empty Entity Creation and Destructon", "[engine::ecs::world]")
{
    LITL::Engine::ECS::World world;
    LITL::Engine::ECS::Entity entity = world.createImmediate();

    REQUIRE(world.isAlive(entity) == true);

    world.destroyImmediate(entity);

    REQUIRE(world.isAlive(entity) == false);
}

/*
TEST_CASE("Entity Add Component", "[engine::ecs::world]")
{
    LITL::Engine::ECS::World world;
    LITL::Engine::ECS::Entity entity = world.createImmediate();

    // ... todo EntityRecord::archetype shouldn't be null for an alive component ...
    // ... need an "EmptyArchetype" to place them into ... right? ...
    world.addComponentImmediate<Foo>(entity);

    REQUIRE(world.componentCount(entity) == 1);

    world.addComponentImmediate<Bar>(entity);

    REQUIRE(world.componentCount(entity) == 2);

    world.addComponentImmediate<Foo>(entity);

    REQUIRE(world.componentCount(entity) == 2);
}
*/