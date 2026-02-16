#include <catch2/catch_test_macros.hpp>
#include "litl-engine/ecs/archetype/chunk.hpp"

TEST_CASE("Header Creation", "[engine::ecs::chunk]")
{
    const uint32_t capacity = 100;

    LITL::Engine::ECS::ChunkDescriptor descriptor{
        .archetype = nullptr,
        .chunkElementCapacity = capacity
    };

    LITL::Engine::ECS::Chunk chunk0(0, &descriptor);
    LITL::Engine::ECS::Chunk chunk1(1, &descriptor);

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