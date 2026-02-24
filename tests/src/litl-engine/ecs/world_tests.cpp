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

    // Get
    auto fooOpt = world.getComponent<Foo>(entity);
    REQUIRE(fooOpt != std::nullopt);
    REQUIRE(fooOpt->a != updatedValue);
    
    // Set
    fooOpt->a = updatedValue;
    world.setComponent<Foo>(entity, fooOpt.value());

    // Get again
    fooOpt = world.getComponent<Foo>(entity);
    REQUIRE(fooOpt != std::nullopt);
    REQUIRE(fooOpt->a == updatedValue);

    world.destroyImmediate(entity);
}

TEST_CASE("Modify Multiple Components", "[engine::ecs::world]")
{
    LITL::Engine::ECS::World world;
    LITL::Engine::ECS::Entity entity = world.createImmediate();

    world.addComponentsImmediate<Foo, Bar>(entity);

    Foo foo{ 50 };
    Bar bar{ 100.0f, 150 };

    world.setComponent<Foo>(entity, foo);
    world.setComponent<Bar>(entity, bar);

    std::optional<Foo> fooVal = world.getComponent<Foo>(entity);
    std::optional<Bar> barVal = world.getComponent<Bar>(entity);

    REQUIRE(fooVal != std::nullopt);
    REQUIRE(barVal != std::nullopt);

    REQUIRE(fooVal->a == foo.a);
    REQUIRE(LITL::Math::floatEquals(barVal->a, bar.a) == true);
    REQUIRE(barVal->b == bar.b);

    world.destroyImmediate(entity);
}

TEST_CASE("Modify Multiple Entity Components", "[engine::ecs::world]")
{
    LITL::Engine::ECS::World world;

    constexpr auto entityCount = 10;

    std::vector<LITL::Engine::ECS::Entity> entities;
    entities.reserve(entityCount);

    // Create entities
    for (auto i = 0; i < entityCount; ++i)
    {
        entities.push_back(world.createImmediate());
        world.addComponentsImmediate<Foo, Bar>(entities[i]);
    }

    Foo foo{};
    Bar bar{};

    // Update entities
    for (auto entity : entities)
    {
        world.setComponent<Foo>(entity, foo);
        world.setComponent<Bar>(entity, bar);

        foo.a++;
        bar.b++;
    }

    // Check updates went through
    for (auto i = 0; i < entityCount; ++i)
    {
        auto entity = entities[i];
        auto entityFoo = world.getComponent<Foo>(entity);
        auto entityBar = world.getComponent<Bar>(entity);

        REQUIRE(entityFoo->a == i);
        REQUIRE(entityBar->b == i);
    }

    for (auto entity : entities)
    {
        world.destroyImmediate(entity);
    }
}