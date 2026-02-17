#include <catch2/catch_test_macros.hpp>
#include "litl-engine/ecs/archetype/chunk.hpp"

struct Foo
{
    uint32_t a;
};

struct Bar
{
    float a;
    uint32_t b;
};

TEST_CASE("Chunk Layout", "[engine::ecs::chunk]")
{
    LITL::Engine::ECS::ChunkLayout layout;

    LITL::Engine::ECS::populateChunkLayout<Foo, Bar>(&layout);

    REQUIRE(layout.componentTypeCount == 2);
}

TEST_CASE("Header Creation", "[engine::ecs::chunk]")
{
    const uint32_t capacity = 100;

    LITL::Engine::ECS::ChunkLayout layout;
    layout.chunkElementCapacity = 2;

    LITL::Engine::ECS::Chunk chunk0(0, &layout);
    LITL::Engine::ECS::Chunk chunk1(1, &layout);

    auto chunk0Header = chunk0.getHeader();
    REQUIRE(chunk0Header->count == 0);
    REQUIRE(chunk0Header->capacity == capacity);
    REQUIRE(chunk0Header->chunkIndex == 0);
    REQUIRE(chunk0Header->version == 0);

    auto chunk1Header = chunk1.getHeader();
    REQUIRE(chunk1Header->count == 0);
    REQUIRE(chunk1Header->capacity == capacity);
    REQUIRE(chunk1Header->chunkIndex == 1);
    REQUIRE(chunk1Header->version == 0);
}