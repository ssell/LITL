#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "litl-engine/ecs/archetype/archetype.hpp"

struct Foo
{
    uint32_t a;
};

struct Bar
{
    float a;
    uint32_t b;
};

TEST_CASE("Archetype Key", "[engine::ecs::archetype]")
{
    std::unique_ptr<LITL::Engine::ECS::Archetype> archetypeFoo(LITL::Engine::ECS::Archetype::build<Foo>());
    std::unique_ptr<LITL::Engine::ECS::Archetype> archetypeFooBar(LITL::Engine::ECS::Archetype::build<Foo, Bar>());
    std::unique_ptr<LITL::Engine::ECS::Archetype> archetypeBarFoo(LITL::Engine::ECS::Archetype::build<Bar, Foo>());

    REQUIRE(archetypeFoo->key() != archetypeFooBar->key());
    REQUIRE(archetypeFoo->key() != archetypeFooBar->key());
    REQUIRE(archetypeFoo->key() != archetypeBarFoo->key());
    REQUIRE(archetypeFooBar->key() == archetypeBarFoo->key());
}

TEST_CASE("Build Archetype", "[engine::ecs::archetype]")
{
    std::unique_ptr<LITL::Engine::ECS::Archetype> archetype(LITL::Engine::ECS::Archetype::build<Foo, Bar>());
    auto archetypeChunkLayout = archetype->layout();

    REQUIRE(archetypeChunkLayout->archetype == archetype.get());
    REQUIRE(archetypeChunkLayout->componentTypeCount == 2);
    REQUIRE(archetypeChunkLayout->chunkElementCapacity > 0);
}