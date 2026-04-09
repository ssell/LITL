#include <algorithm>

#include "tests.hpp"
#include "litl-ecs/tests-common.hpp"
#include "litl-ecs/archetype/archetype.hpp"
#include "litl-ecs/archetype/archetypeRegistry.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("ArchetypeRegistry::get (Static)", "[ecs::archetype]")
    {
        auto archetypeFoo = ArchetypeRegistry::get<Foo>();
        auto archetypeFooFoo = ArchetypeRegistry::get<Foo, Foo>();   // duplicate component should resolve to <Foo>
        auto archetypeFooBar = ArchetypeRegistry::get<Foo, Bar>();
        auto archetypeBarFoo = ArchetypeRegistry::get<Bar, Foo>();   // different order should resolve to <Foo, Bar>

        REQUIRE(archetypeFoo == archetypeFooFoo);
        REQUIRE(archetypeFoo != archetypeFooBar);
        REQUIRE(archetypeFooBar == archetypeBarFoo);
        REQUIRE(archetypeFoo->id() != archetypeFooBar->id());
        REQUIRE(archetypeFoo->componentHash() != archetypeFooBar->componentHash());

        auto archetypeFun = ArchetypeRegistry::get<Bar, Bar, Foo, Foo, Foo, Bar, Foo, Foo, Bar, Foo>();

        REQUIRE(archetypeFun == archetypeFooBar);

        REQUIRE(archetypeFoo->componentCount() == 1);
        REQUIRE(archetypeFooBar->componentCount() == 2);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("ArchetypeRegistry::get (Dynamic)", "[ecs::archetype]")
    {
        auto fooComponent = ComponentDescriptor::get<Foo>();
        auto barComponent = ComponentDescriptor::get<Bar>();

        auto archetypeFooS = ArchetypeRegistry::get<Foo>();
        auto archetypeFooD = ArchetypeRegistry::getByComponents({ fooComponent->id });
        auto archetypeFooFooD = ArchetypeRegistry::getByComponents({ fooComponent->id, fooComponent->id });

        REQUIRE(archetypeFooS == archetypeFooD);
        REQUIRE(archetypeFooD == archetypeFooFooD);

        auto archetypeFooBarS = ArchetypeRegistry::get<Foo, Bar>();
        auto archetypeFooBarD = ArchetypeRegistry::getByComponents({ fooComponent->id, barComponent->id });
        auto archetypeBarFooD = ArchetypeRegistry::getByComponents({ barComponent->id, fooComponent->id });

        REQUIRE(archetypeFooBarS == archetypeFooBarD);
        REQUIRE(archetypeFooBarD == archetypeBarFooD);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("ArchetypeRegistry::getById", "[ecs::archetype]")
    {
        auto archetypeFoo = ArchetypeRegistry::get<Foo>();
        auto archetypeFooBar = ArchetypeRegistry::get<Foo, Bar>();

        REQUIRE(ArchetypeRegistry::getById(archetypeFoo->id()) == archetypeFoo);
        REQUIRE(ArchetypeRegistry::getById(archetypeFooBar->id()) == archetypeFooBar);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("ArchetypeRegistry::getByComponentHash", "[ecs::archetype]")
    {
        auto archetypeFoo = ArchetypeRegistry::get<Foo>();
        auto archetypeFooBar = ArchetypeRegistry::get<Foo, Bar>();

        REQUIRE(ArchetypeRegistry::getByComponentHash(archetypeFoo->componentHash()) == archetypeFoo);
        REQUIRE(ArchetypeRegistry::getByComponentHash(archetypeFooBar->componentHash()) == archetypeFooBar);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Empty Archetype", "[ecs::archetype]")
    {
        auto emptyArchetype = ArchetypeRegistry::getByComponents({});

        REQUIRE(emptyArchetype != nullptr);
        REQUIRE(emptyArchetype->componentCount() == 0);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Archetype Has Component", "[ecs::archetype]")
    {
        auto* archetypeFoo = ArchetypeRegistry::get<Foo>();
        auto* archetypeFooBar = ArchetypeRegistry::get<Foo, Bar>();

        const auto fooId = ComponentDescriptor::get<Foo>()->id;
        const auto barId = ComponentDescriptor::get<Bar>()->id;

        REQUIRE(archetypeFoo->hasComponent<Foo>() == true);
        REQUIRE(archetypeFoo->hasComponent<Bar>() == false);
        REQUIRE(archetypeFoo->hasComponent(fooId) == true);
        REQUIRE(archetypeFoo->hasComponent(barId) == false);

        REQUIRE(archetypeFooBar->hasComponent<Foo>() == true);
        REQUIRE(archetypeFooBar->hasComponent<Bar>() == true);
        REQUIRE(archetypeFooBar->hasComponent(fooId) == true);
        REQUIRE(archetypeFooBar->hasComponent(barId) == true);
    } END_LITL_TEST_CASE

    namespace NewArchetypesTest
    {
        struct Apple {};
        struct Orange {};
    }

    LITL_TEST_CASE("Report New Archetypes", "[ecs::archetype]")
    {
        ArchetypeRegistry::fetchNewArchetypes();     // clear out awaiting new archetypes

        auto appleArchetype = ArchetypeRegistry::get<NewArchetypesTest::Apple>();
        auto orangeArchetype = ArchetypeRegistry::get<NewArchetypesTest::Orange>();
        auto appleOrangeArchetype = ArchetypeRegistry::get<NewArchetypesTest::Apple, NewArchetypesTest::Orange>();
        auto orangeAppleArchetype = ArchetypeRegistry::get<NewArchetypesTest::Orange, NewArchetypesTest::Apple>();

        REQUIRE(appleOrangeArchetype == orangeAppleArchetype);

        auto newArchetypes = ArchetypeRegistry::fetchNewArchetypes();

        REQUIRE(newArchetypes.size() == 3);

        REQUIRE(std::find(newArchetypes.begin(), newArchetypes.end(), appleArchetype->id()) != newArchetypes.end());
        REQUIRE(std::find(newArchetypes.begin(), newArchetypes.end(), orangeArchetype->id()) != newArchetypes.end());
        REQUIRE(std::find(newArchetypes.begin(), newArchetypes.end(), appleOrangeArchetype->id()) != newArchetypes.end());
    } END_LITL_TEST_CASE
}

REGISTER_TYPE_NAME(litl::tests::NewArchetypesTest::Apple);
REGISTER_TYPE_NAME(litl::tests::NewArchetypesTest::Orange);