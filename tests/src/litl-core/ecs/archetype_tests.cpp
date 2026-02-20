#include <catch2/catch_test_macros.hpp>
#include "litl-engine/ecs/archetype/archetype.hpp"
#include "litl-engine/ecs/archetype/archetypeRegistry.hpp"

struct Foo
{
    uint32_t a;
};

struct Bar
{
    float a;
    uint32_t b;
};

TEST_CASE("ArchetypeRegistry::get (Static)", "[engine::ecs::archetype]")
{
    auto archetypeFoo    = LITL::Engine::ECS::ArchetypeRegistry::get<Foo>();
    auto archetypeFooFoo = LITL::Engine::ECS::ArchetypeRegistry::get<Foo, Foo>();   // duplicate component should resolve to <Foo>
    auto archetypeFooBar = LITL::Engine::ECS::ArchetypeRegistry::get<Foo, Bar>();
    auto archetypeBarFoo = LITL::Engine::ECS::ArchetypeRegistry::get<Bar, Foo>();   // different order should resolve to <Foo, Bar>

    REQUIRE(archetypeFoo == archetypeFooFoo);
    REQUIRE(archetypeFoo != archetypeFooBar);
    REQUIRE(archetypeFooBar == archetypeBarFoo);
    REQUIRE(archetypeFoo->registryIndex() != archetypeFooBar->registryIndex());
    REQUIRE(archetypeFoo->componentHash() != archetypeFooBar->componentHash());

    auto archetypeFun = LITL::Engine::ECS::ArchetypeRegistry::get<Bar, Bar, Foo, Foo, Foo, Bar, Foo, Foo, Bar, Foo>();

    REQUIRE(archetypeFun == archetypeFooBar);

    REQUIRE(archetypeFoo->componentCount() == 1);
    REQUIRE(archetypeFooBar->componentCount() == 2);
}

TEST_CASE("ArchetypeRegistry::get (Dynamic)", "[engine::ecs::archetype]")
{
    auto fooComponent = LITL::Engine::ECS::ComponentDescriptor::get<Foo>();
    auto barComponent = LITL::Engine::ECS::ComponentDescriptor::get<Bar>();

    auto archetypeFooS = LITL::Engine::ECS::ArchetypeRegistry::get<Foo>();
    auto archetypeFooD = LITL::Engine::ECS::ArchetypeRegistry::getByComponents({ fooComponent->id });
    auto archetypeFooFooD = LITL::Engine::ECS::ArchetypeRegistry::getByComponents({ fooComponent->id, fooComponent->id });

    REQUIRE(archetypeFooS == archetypeFooD);
    REQUIRE(archetypeFooD == archetypeFooFooD);

    auto archetypeFooBarS = LITL::Engine::ECS::ArchetypeRegistry::get<Foo, Bar>();
    auto archetypeFooBarD = LITL::Engine::ECS::ArchetypeRegistry::getByComponents({ fooComponent->id, barComponent->id });
    auto archetypeBarFooD = LITL::Engine::ECS::ArchetypeRegistry::getByComponents({ barComponent->id, fooComponent->id });

    REQUIRE(archetypeFooBarS == archetypeFooBarD);
    REQUIRE(archetypeFooBarD == archetypeBarFooD);
}

TEST_CASE("ArchetypeRegistry::getByIndex", "[engine::ecs::archetype]")
{
    auto archetypeFoo = LITL::Engine::ECS::ArchetypeRegistry::get<Foo>();
    auto archetypeFooBar = LITL::Engine::ECS::ArchetypeRegistry::get<Foo, Bar>();

    REQUIRE(LITL::Engine::ECS::ArchetypeRegistry::getByIndex(archetypeFoo->registryIndex()) == archetypeFoo);
    REQUIRE(LITL::Engine::ECS::ArchetypeRegistry::getByIndex(archetypeFooBar->registryIndex()) == archetypeFooBar);
}

TEST_CASE("ArchetypeRegistry::getByComponentHash", "[engine::ecs::archetype]")
{
    auto archetypeFoo = LITL::Engine::ECS::ArchetypeRegistry::get<Foo>();
    auto archetypeFooBar = LITL::Engine::ECS::ArchetypeRegistry::get<Foo, Bar>();

    REQUIRE(LITL::Engine::ECS::ArchetypeRegistry::getByComponentHash(archetypeFoo->componentHash()) == archetypeFoo);
    REQUIRE(LITL::Engine::ECS::ArchetypeRegistry::getByComponentHash(archetypeFooBar->componentHash()) == archetypeFooBar);
}

TEST_CASE("Empty Archetype", "[engine::ecs::archetype]")
{
    auto emptyArchetype = LITL::Engine::ECS::ArchetypeRegistry::getByComponents({});

    REQUIRE(emptyArchetype != nullptr);
    REQUIRE(emptyArchetype->componentCount() == 0);
}