#include <catch2/catch_test_macros.hpp>

#include "litl-engine/ecs/ecsCommon.hpp"
#include "litl-engine/ecs/world.hpp"
#include "litl-engine/ecs/archetype/archetype.hpp"
#include "litl-engine/ecs/archetype/archetypeRegistry.hpp"

TEST_CASE("Empty Entity Creation and Destructon", "[engine::ecs::world]")
{
    LITL::Engine::ECS::World world;
    LITL::Engine::ECS::Entity entity = world.createImmediate();

    REQUIRE(world.isAlive(entity) == true);

    world.destroyImmediate(entity);

    REQUIRE(world.isAlive(entity) == false);
}

TEST_CASE("Entity Add/Remove Component", "[engine::ecs::world]")
{
    LITL::Engine::ECS::Archetype* fooArchetype = LITL::Engine::ECS::ArchetypeRegistry::get<Foo>();
    LITL::Engine::ECS::Archetype* fooBarArchetype = LITL::Engine::ECS::ArchetypeRegistry::get<Foo, Bar>();

    REQUIRE(fooArchetype->entityCount() == 0);                      // If these checks fail then another test has left ECS in a dirty state.
    REQUIRE(fooBarArchetype->entityCount() == 0);

    LITL::Engine::ECS::World world;
    LITL::Engine::ECS::Entity entity = world.createImmediate();

    // entity = [Foo]
    world.addComponentImmediate<Foo>(entity);

    REQUIRE(fooArchetype->entityCount() == 1);
    REQUIRE(fooBarArchetype->entityCount() == 0);
    REQUIRE(world.componentCount(entity) == 1);

    // entity = [Foo,Bar]
    world.addComponentImmediate<Bar>(entity);

    REQUIRE(fooArchetype->entityCount() == 0);
    REQUIRE(fooBarArchetype->entityCount() == 1);
    REQUIRE(world.componentCount(entity) == 2);

    // entity = [Foo,Bar]
    world.addComponentImmediate<Foo>(entity);

    REQUIRE(fooArchetype->entityCount() == 0);
    REQUIRE(fooBarArchetype->entityCount() == 1);
    REQUIRE(world.componentCount(entity) == 2);

    // entity = [Foo]
    world.removeComponentImmediate<Bar>(entity);

    REQUIRE(fooArchetype->entityCount() == 1);
    REQUIRE(fooBarArchetype->entityCount() == 0);
    REQUIRE(world.componentCount(entity) == 1);

    // entity = []
    world.removeComponentImmediate<Foo>(entity);

    REQUIRE(fooArchetype->entityCount() == 0);
    REQUIRE(fooBarArchetype->entityCount() == 0);
    REQUIRE(world.componentCount(entity) == 0);

    // entity = dead (but still in [])
    world.destroyImmediate(entity);

    REQUIRE(fooArchetype->entityCount() == 0);
    REQUIRE(fooBarArchetype->entityCount() == 0);
    REQUIRE(world.isAlive(entity) == false);
}

TEST_CASE("Entity Add / Remove Components", "[engine::ecs::world]")
{
    LITL::Engine::ECS::Archetype* fooBarArchetype = LITL::Engine::ECS::ArchetypeRegistry::get<Foo, Bar>();

    LITL::Engine::ECS::World world;
    LITL::Engine::ECS::Entity entity = world.createImmediate();

    world.addComponentsImmediate<Foo, Bar>(entity);

    REQUIRE(fooBarArchetype->entityCount() == 1);
    REQUIRE(world.componentCount(entity) == 2);

    world.addComponentsImmediate<Bar, Bar, Foo, Foo>(entity);

    REQUIRE(fooBarArchetype->entityCount() == 1);
    REQUIRE(world.componentCount(entity) == 2);

    world.removeComponentsImmediate<Foo, Bar>(entity);

    REQUIRE(fooBarArchetype->entityCount() == 0);
    REQUIRE(world.componentCount(entity) == 0);

    world.destroyImmediate(entity);

    REQUIRE(fooBarArchetype->entityCount() == 0);
    REQUIRE(world.isAlive(entity) == false);
}

TEST_CASE("Destroy Entity with Components", "[engine::ecs::world]")
{
    LITL::Engine::ECS::Archetype* fooBarArchetype = LITL::Engine::ECS::ArchetypeRegistry::get<Foo, Bar>();

    LITL::Engine::ECS::World world;
    LITL::Engine::ECS::Entity entity = world.createImmediate();

    world.addComponentsImmediate<Foo, Bar>(entity);

    REQUIRE(fooBarArchetype->entityCount() == 1);
    REQUIRE(world.componentCount(entity) == 2);

    world.destroyImmediate(entity);

    REQUIRE(fooBarArchetype->entityCount() == 0);
    REQUIRE(world.isAlive(entity) == false);
}

namespace ComponentCtorDtorCallTest
{
    static uint32_t CtorDtorTestCount = 0;

    struct CtorDtorTestComponent
    {
        CtorDtorTestComponent()
        {
            CtorDtorTestCount++;
        }

        ~CtorDtorTestComponent()
        {
            CtorDtorTestCount--;
        }

        uint32_t value;
    };
}

TEST_CASE("Component Ctor Dtor Call", "[engine::ecs::world]")
{
    LITL::Engine::ECS::World world;
    LITL::Engine::ECS::Entity entity0 = world.createImmediate();
    LITL::Engine::ECS::Entity entity1 = world.createImmediate();

    REQUIRE(ComponentCtorDtorCallTest::CtorDtorTestCount == 0);

    world.addComponentImmediate<ComponentCtorDtorCallTest::CtorDtorTestComponent>(entity0);

    REQUIRE(ComponentCtorDtorCallTest::CtorDtorTestCount == 1);

    world.addComponentImmediate<ComponentCtorDtorCallTest::CtorDtorTestComponent>(entity1);

    REQUIRE(ComponentCtorDtorCallTest::CtorDtorTestCount == 2);

    world.destroyImmediate(entity1);

    REQUIRE(ComponentCtorDtorCallTest::CtorDtorTestCount == 1);

    world.removeComponentImmediate<ComponentCtorDtorCallTest::CtorDtorTestComponent>(entity0);

    REQUIRE(ComponentCtorDtorCallTest::CtorDtorTestCount == 0);

    world.destroyImmediate(entity1);

    REQUIRE(ComponentCtorDtorCallTest::CtorDtorTestCount == 0);
}

TEST_CASE("Modify Component", "[engine::ecs::world]")
{
    LITL::Engine::ECS::World world;
    LITL::Engine::ECS::Entity entity = world.createImmediate();

    const uint32_t updatedValue = 55;

    world.addComponentImmediate<Foo>(entity);

   // auto fooOpt = world.getComponent<Foo>(entity);
   //
   // REQUIRE(fooOpt != std::nullopt);
   //
   //
   //
   //
   // world.destroyImmediate(entity);
}