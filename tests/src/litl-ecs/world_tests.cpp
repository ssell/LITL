#include <catch2/catch_test_macros.hpp>

#include "litl-ecs/common.hpp"
#include "litl-ecs/world.hpp"
#include "litl-ecs/archetype/archetype.hpp"
#include "litl-ecs/archetype/archetypeRegistry.hpp"

TEST_CASE("Empty Entity Creation and Destructon", "[ecs::world]")
{
    LITL::ECS::World world;
    LITL::ECS::Entity entity = world.createImmediate();

    REQUIRE(world.isAlive(entity) == true);

    world.destroyImmediate(entity);

    REQUIRE(world.isAlive(entity) == false);
}

TEST_CASE("Add Many Entities", "[ecs::world]")
{
    constexpr auto entityCount = 2048; // fill up multiple chunks worth

    LITL::ECS::World world;
    LITL::ECS::Archetype* fooBarArchetype = LITL::ECS::ArchetypeRegistry::get<Foo, Bar>();

    std::vector<LITL::ECS::Entity> entities;
    entities.reserve(entityCount);

    const auto initialFooBarCount = fooBarArchetype->entityCount();
    const auto initialEmptyCount = LITL::ECS::ArchetypeRegistry::Empty()->entityCount();

    for (auto i = 0; i < entityCount; ++i)
    {
        entities.push_back(world.createImmediate());
        world.addComponentsImmediate<Foo, Bar>(entities[i]);
    }

    REQUIRE(fooBarArchetype->entityCount() == (initialFooBarCount + entityCount));
    REQUIRE(LITL::ECS::ArchetypeRegistry::Empty()->entityCount() == initialEmptyCount);

    for (auto i = 0; i < entityCount; ++i)
    {
        world.destroyImmediate(entities[i]);
    }

    REQUIRE(fooBarArchetype->entityCount() == initialFooBarCount);
    REQUIRE(LITL::ECS::ArchetypeRegistry::Empty()->entityCount() == (initialEmptyCount + entityCount));
}

TEST_CASE("Entity Add/Remove Component", "[ecs::world]")
{
    LITL::ECS::World world;
    LITL::ECS::Archetype* fooArchetype = LITL::ECS::ArchetypeRegistry::get<Foo>();
    LITL::ECS::Archetype* fooBarArchetype = LITL::ECS::ArchetypeRegistry::get<Foo, Bar>();
    LITL::ECS::Entity entity = world.createImmediate();

    const auto initialFooCount = fooArchetype->entityCount();
    const auto initialFooBarCount = fooBarArchetype->entityCount();

    // entity = [Foo]
    world.addComponentImmediate<Foo>(entity);

    REQUIRE(fooArchetype->entityCount() == (initialFooCount + 1));
    REQUIRE(fooBarArchetype->entityCount() == initialFooBarCount);
    REQUIRE(world.componentCount(entity) == 1);

    // entity = [Foo,Bar]
    world.addComponentImmediate<Bar>(entity);

    REQUIRE(fooArchetype->entityCount() == initialFooCount);
    REQUIRE(fooBarArchetype->entityCount() == (initialFooBarCount + 1));
    REQUIRE(world.componentCount(entity) == 2);

    // entity = [Foo,Bar]
    world.addComponentImmediate<Foo>(entity);

    REQUIRE(fooArchetype->entityCount() == initialFooCount);
    REQUIRE(fooBarArchetype->entityCount() == (initialFooBarCount + 1));
    REQUIRE(world.componentCount(entity) == 2);

    // entity = [Foo]
    world.removeComponentImmediate<Bar>(entity);

    REQUIRE(fooArchetype->entityCount() == (initialFooCount + 1));
    REQUIRE(fooBarArchetype->entityCount() == initialFooBarCount);
    REQUIRE(world.componentCount(entity) == 1);

    // entity = []
    world.removeComponentImmediate<Foo>(entity);

    REQUIRE(fooArchetype->entityCount() == initialFooCount);
    REQUIRE(fooBarArchetype->entityCount() == initialFooBarCount);
    REQUIRE(world.componentCount(entity) == 0);

    // entity = dead (but still in [])
    world.destroyImmediate(entity);

    REQUIRE(fooArchetype->entityCount() == initialFooCount);
    REQUIRE(fooBarArchetype->entityCount() == initialFooBarCount);
    REQUIRE(world.isAlive(entity) == false);
}

TEST_CASE("Entity Add / Remove Components", "[ecs::world]")
{
    LITL::ECS::Archetype* fooBarArchetype = LITL::ECS::ArchetypeRegistry::get<Foo, Bar>();

    LITL::ECS::World world;
    LITL::ECS::Entity entity = world.createImmediate();

    const auto initialFooBarCount = fooBarArchetype->entityCount();

    world.addComponentsImmediate<Foo, Bar>(entity);

    REQUIRE(fooBarArchetype->entityCount() == (initialFooBarCount + 1));
    REQUIRE(world.componentCount(entity) == 2);

    world.addComponentsImmediate<Bar, Bar, Foo, Foo>(entity);

    REQUIRE(fooBarArchetype->entityCount() == (initialFooBarCount + 1));
    REQUIRE(world.componentCount(entity) == 2);

    world.removeComponentsImmediate<Foo, Bar>(entity);

    REQUIRE(fooBarArchetype->entityCount() == initialFooBarCount);
    REQUIRE(world.componentCount(entity) == 0);

    world.destroyImmediate(entity);

    REQUIRE(fooBarArchetype->entityCount() == initialFooBarCount);
    REQUIRE(world.isAlive(entity) == false);
}

TEST_CASE("Entity Add Components With Value", "[ecs::world]")
{
    LITL::ECS::World world;
    LITL::ECS::Entity entity0 = world.createImmediate();
    LITL::ECS::Entity entity1 = world.createImmediate();

    world.addComponentImmediate(entity0, Foo{ 1337 });
    world.addComponentsImmediate(entity1, Foo{ 1338 }, Bar{ 10.0f, 100 });

    auto entity0Foo = world.getComponent<Foo>(entity0);
    auto entity1Foo = world.getComponent<Foo>(entity1);
    auto entity1Bar = world.getComponent<Bar>(entity1);

    REQUIRE(entity0Foo != std::nullopt);
    REQUIRE(entity1Foo != std::nullopt);
    REQUIRE(entity1Foo != std::nullopt);

    REQUIRE(entity0Foo->a == 1337);
    REQUIRE(entity1Foo->a == 1338);
    REQUIRE(entity1Bar->b == 100);
    REQUIRE(LITL::Math::floatEquals(entity1Bar->a, 10.0f));

    world.destroyImmediate(entity1);
    world.destroyImmediate(entity0);
}

TEST_CASE("Destroy Entity with Components", "[ecs::world]")
{
    LITL::ECS::Archetype* fooBarArchetype = LITL::ECS::ArchetypeRegistry::get<Foo, Bar>();

    LITL::ECS::World world;
    LITL::ECS::Entity entity = world.createImmediate();
    const auto initialFooBarCount = fooBarArchetype->entityCount();

    world.addComponentsImmediate<Foo, Bar>(entity);

    REQUIRE(fooBarArchetype->entityCount() == (initialFooBarCount + 1));
    REQUIRE(world.componentCount(entity) == 2);

    world.destroyImmediate(entity);

    REQUIRE(fooBarArchetype->entityCount() == initialFooBarCount);
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

TEST_CASE("Component Ctor Dtor Call", "[ecs::world]")
{
    LITL::ECS::World world;
    LITL::ECS::Entity entity0 = world.createImmediate();
    LITL::ECS::Entity entity1 = world.createImmediate();

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

TEST_CASE("Modify Component", "[ecs::world]")
{
    LITL::ECS::World world;
    LITL::ECS::Entity entity = world.createImmediate();

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

TEST_CASE("Modify Multiple Components", "[ecs::world]")
{
    LITL::ECS::World world;
    LITL::ECS::Entity entity = world.createImmediate();

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

TEST_CASE("Modify Multiple Entity Components", "[ecs::world]")
{
    LITL::ECS::World world;

    constexpr auto entityCount = 10;

    std::vector<LITL::ECS::Entity> entities;
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