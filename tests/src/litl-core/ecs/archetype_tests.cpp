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

TEST_CASE("Archetype Registry", "[engine::ecs::archetype]")
{
    auto archetypeFoo    = LITL::Engine::ECS::ArchetypeRegistry::get<Foo>();
    auto archetypeFooFoo = LITL::Engine::ECS::ArchetypeRegistry::get<Foo, Foo>();
    auto archetypeFooBar = LITL::Engine::ECS::ArchetypeRegistry::get<Foo, Bar>();
    auto archetypeBarFoo = LITL::Engine::ECS::ArchetypeRegistry::get<Bar, Foo>();

    REQUIRE(archetypeFoo == archetypeFooFoo);
    REQUIRE(archetypeFoo != archetypeFooBar);
    REQUIRE(archetypeFooBar == archetypeBarFoo);
    REQUIRE(archetypeFoo->key() != archetypeFooBar->key());
}