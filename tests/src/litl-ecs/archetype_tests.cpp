#include <catch2/catch_test_macros.hpp>
#include "litl-ecs/common.hpp"
#include "litl-ecs/archetype/archetype.hpp"
#include "litl-ecs/archetype/archetypeRegistry.hpp"


TEST_CASE("ArchetypeRegistry::get (Static)", "[engine::ecs::archetype]")
{
    auto archetypeFoo    = LITL::ECS::ArchetypeRegistry::get<Foo>();
    auto archetypeFooFoo = LITL::ECS::ArchetypeRegistry::get<Foo, Foo>();   // duplicate component should resolve to <Foo>
    auto archetypeFooBar = LITL::ECS::ArchetypeRegistry::get<Foo, Bar>();
    auto archetypeBarFoo = LITL::ECS::ArchetypeRegistry::get<Bar, Foo>();   // different order should resolve to <Foo, Bar>

    REQUIRE(archetypeFoo == archetypeFooFoo);
    REQUIRE(archetypeFoo != archetypeFooBar);
    REQUIRE(archetypeFooBar == archetypeBarFoo);
    REQUIRE(archetypeFoo->id() != archetypeFooBar->id());
    REQUIRE(archetypeFoo->componentHash() != archetypeFooBar->componentHash());

    auto archetypeFun = LITL::ECS::ArchetypeRegistry::get<Bar, Bar, Foo, Foo, Foo, Bar, Foo, Foo, Bar, Foo>();

    REQUIRE(archetypeFun == archetypeFooBar);

    REQUIRE(archetypeFoo->componentCount() == 1);
    REQUIRE(archetypeFooBar->componentCount() == 2);
}

TEST_CASE("ArchetypeRegistry::get (Dynamic)", "[engine::ecs::archetype]")
{
    auto fooComponent = LITL::ECS::ComponentDescriptor::get<Foo>();
    auto barComponent = LITL::ECS::ComponentDescriptor::get<Bar>();

    auto archetypeFooS = LITL::ECS::ArchetypeRegistry::get<Foo>();
    auto archetypeFooD = LITL::ECS::ArchetypeRegistry::getByComponents({ fooComponent->id });
    auto archetypeFooFooD = LITL::ECS::ArchetypeRegistry::getByComponents({ fooComponent->id, fooComponent->id });

    REQUIRE(archetypeFooS == archetypeFooD);
    REQUIRE(archetypeFooD == archetypeFooFooD);

    auto archetypeFooBarS = LITL::ECS::ArchetypeRegistry::get<Foo, Bar>();
    auto archetypeFooBarD = LITL::ECS::ArchetypeRegistry::getByComponents({ fooComponent->id, barComponent->id });
    auto archetypeBarFooD = LITL::ECS::ArchetypeRegistry::getByComponents({ barComponent->id, fooComponent->id });

    REQUIRE(archetypeFooBarS == archetypeFooBarD);
    REQUIRE(archetypeFooBarD == archetypeBarFooD);
}

TEST_CASE("ArchetypeRegistry::getById", "[engine::ecs::archetype]")
{
    auto archetypeFoo = LITL::ECS::ArchetypeRegistry::get<Foo>();
    auto archetypeFooBar = LITL::ECS::ArchetypeRegistry::get<Foo, Bar>();

    REQUIRE(LITL::ECS::ArchetypeRegistry::getById(archetypeFoo->id()) == archetypeFoo);
    REQUIRE(LITL::ECS::ArchetypeRegistry::getById(archetypeFooBar->id()) == archetypeFooBar);
}

TEST_CASE("ArchetypeRegistry::getByComponentHash", "[engine::ecs::archetype]")
{
    auto archetypeFoo = LITL::ECS::ArchetypeRegistry::get<Foo>();
    auto archetypeFooBar = LITL::ECS::ArchetypeRegistry::get<Foo, Bar>();

    REQUIRE(LITL::ECS::ArchetypeRegistry::getByComponentHash(archetypeFoo->componentHash()) == archetypeFoo);
    REQUIRE(LITL::ECS::ArchetypeRegistry::getByComponentHash(archetypeFooBar->componentHash()) == archetypeFooBar);
}

TEST_CASE("Empty Archetype", "[engine::ecs::archetype]")
{
    auto emptyArchetype = LITL::ECS::ArchetypeRegistry::getByComponents({});

    REQUIRE(emptyArchetype != nullptr);
    REQUIRE(emptyArchetype->componentCount() == 0);
}

TEST_CASE("Archetype Has Component", "[engine::ecs::archetype]")
{
    auto* archetypeFoo = LITL::ECS::ArchetypeRegistry::get<Foo>();
    auto* archetypeFooBar = LITL::ECS::ArchetypeRegistry::get<Foo, Bar>();

    const auto fooId = LITL::ECS::ComponentDescriptor::get<Foo>()->id;
    const auto barId = LITL::ECS::ComponentDescriptor::get<Bar>()->id;

    REQUIRE(archetypeFoo->hasComponent<Foo>() == true);
    REQUIRE(archetypeFoo->hasComponent<Bar>() == false);
    REQUIRE(archetypeFoo->hasComponent(fooId) == true);
    REQUIRE(archetypeFoo->hasComponent(barId) == false);

    REQUIRE(archetypeFooBar->hasComponent<Foo>() == true);
    REQUIRE(archetypeFooBar->hasComponent<Bar>() == true);
    REQUIRE(archetypeFooBar->hasComponent(fooId) == true);
    REQUIRE(archetypeFooBar->hasComponent(barId) == true);
}