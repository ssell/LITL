#include <algorithm>
#include <catch2/catch_test_macros.hpp>

#include "litl-ecs/common.hpp"
#include "litl-ecs/archetype/archetype.hpp"
#include "litl-ecs/archetype/archetypeRegistry.hpp"


TEST_CASE("ArchetypeRegistry::get (Static)", "[ecs::archetype]")
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

TEST_CASE("ArchetypeRegistry::get (Dynamic)", "[ecs::archetype]")
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

TEST_CASE("ArchetypeRegistry::getById", "[ecs::archetype]")
{
    auto archetypeFoo = LITL::ECS::ArchetypeRegistry::get<Foo>();
    auto archetypeFooBar = LITL::ECS::ArchetypeRegistry::get<Foo, Bar>();

    REQUIRE(LITL::ECS::ArchetypeRegistry::getById(archetypeFoo->id()) == archetypeFoo);
    REQUIRE(LITL::ECS::ArchetypeRegistry::getById(archetypeFooBar->id()) == archetypeFooBar);
}

TEST_CASE("ArchetypeRegistry::getByComponentHash", "[ecs::archetype]")
{
    auto archetypeFoo = LITL::ECS::ArchetypeRegistry::get<Foo>();
    auto archetypeFooBar = LITL::ECS::ArchetypeRegistry::get<Foo, Bar>();

    REQUIRE(LITL::ECS::ArchetypeRegistry::getByComponentHash(archetypeFoo->componentHash()) == archetypeFoo);
    REQUIRE(LITL::ECS::ArchetypeRegistry::getByComponentHash(archetypeFooBar->componentHash()) == archetypeFooBar);
}

TEST_CASE("Empty Archetype", "[ecs::archetype]")
{
    auto emptyArchetype = LITL::ECS::ArchetypeRegistry::getByComponents({});

    REQUIRE(emptyArchetype != nullptr);
    REQUIRE(emptyArchetype->componentCount() == 0);
}

TEST_CASE("Archetype Has Component", "[ecs::archetype]")
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

namespace NewArchetypesTest
{
    struct Apple {};
    struct Orange {};
}

TEST_CASE("Report New Archetypes", "[ecs::archetype]")
{
    LITL::ECS::ArchetypeRegistry::fetchNewArchetypes();     // clear out awaiting new archetypes

    auto appleArchetype = LITL::ECS::ArchetypeRegistry::get<NewArchetypesTest::Apple>();
    auto orangeArchetype = LITL::ECS::ArchetypeRegistry::get<NewArchetypesTest::Orange>();
    auto appleOrangeArchetype = LITL::ECS::ArchetypeRegistry::get<NewArchetypesTest::Apple, NewArchetypesTest::Orange>();
    auto orangeAppleArchetype = LITL::ECS::ArchetypeRegistry::get<NewArchetypesTest::Orange, NewArchetypesTest::Apple>();

    REQUIRE(appleOrangeArchetype == orangeAppleArchetype);

    auto newArchetypes = LITL::ECS::ArchetypeRegistry::fetchNewArchetypes();

    REQUIRE(newArchetypes.size() == 3);

    REQUIRE(std::find(newArchetypes.begin(), newArchetypes.end(), appleArchetype->id()) != newArchetypes.end());
    REQUIRE(std::find(newArchetypes.begin(), newArchetypes.end(), orangeArchetype->id()) != newArchetypes.end());
    REQUIRE(std::find(newArchetypes.begin(), newArchetypes.end(), appleOrangeArchetype->id()) != newArchetypes.end());
}