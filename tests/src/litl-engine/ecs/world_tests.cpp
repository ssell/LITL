#include <catch2/catch_test_macros.hpp>
#include "litl-engine/ecs/world.hpp"
#include "litl-engine/ecs/archetype/archetype.hpp"
#include "litl-engine/ecs/archetype/archetypeRegistry.hpp"

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

TEST_CASE("Entity Add Component", "[engine::ecs::world]")
{
    LITL::Engine::ECS::Archetype* emptyArchetype = LITL::Engine::ECS::ArchetypeRegistry::Empty();
    LITL::Engine::ECS::Archetype* fooArchetype = LITL::Engine::ECS::ArchetypeRegistry::get<Foo>();
    LITL::Engine::ECS::Archetype* fooBarArchetype = LITL::Engine::ECS::ArchetypeRegistry::get<Foo, Bar>();

    const auto startEmptyCount = emptyArchetype->entityCount();     // This depends on the entities created in other tests.

    REQUIRE(fooArchetype->entityCount() == 0);                      // If these checks fail then another test has left ECS in a dirty state.
    REQUIRE(fooBarArchetype->entityCount() == 0);

    LITL::Engine::ECS::World world;
    LITL::Engine::ECS::Entity entity = world.createImmediate();

    REQUIRE(emptyArchetype->entityCount() == (startEmptyCount + 1));

    world.addComponentImmediate<Foo>(entity);

    REQUIRE(emptyArchetype->entityCount() == startEmptyCount);
    REQUIRE(fooArchetype->entityCount() == 1);
    REQUIRE(fooBarArchetype->entityCount() == 0);
    REQUIRE(world.componentCount(entity) == 1);

    world.addComponentImmediate<Bar>(entity);

    REQUIRE(emptyArchetype->entityCount() == startEmptyCount);
    REQUIRE(fooArchetype->entityCount() == 0);
    REQUIRE(fooBarArchetype->entityCount() == 1);
    REQUIRE(world.componentCount(entity) == 2);

    world.addComponentImmediate<Foo>(entity);

    REQUIRE(emptyArchetype->entityCount() == startEmptyCount);
    REQUIRE(fooArchetype->entityCount() == 0);
    REQUIRE(fooBarArchetype->entityCount() == 1);
    REQUIRE(world.componentCount(entity) == 2);
}