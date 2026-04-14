#include "tests.hpp"
#include "litl-ecs/tests-common.hpp"
#include "litl-ecs/archetype/chunk.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("Chunk Layout", "[ecs::chunk]")
    {
        ChunkLayout layoutFooBar;
        populateChunkLayout<Foo, Bar>(&layoutFooBar);

        ChunkLayout layoutBarFoo;
        populateChunkLayout<Bar, Foo>(&layoutBarFoo);

        REQUIRE(layoutFooBar.componentTypeCount == 2);
        REQUIRE(layoutBarFoo.componentTypeCount == 2);

        // Even though they are declared in opposite orders, these should resolve to the same layout.
        REQUIRE(layoutFooBar.componentOrder[0] == layoutBarFoo.componentOrder[0]);
        REQUIRE(layoutFooBar.componentOrder[1] == layoutBarFoo.componentOrder[1]);

        REQUIRE(layoutFooBar.entityCapacity > 0);
        REQUIRE(layoutFooBar.entityCapacity == layoutBarFoo.entityCapacity);

        // ... todo add more concrete tests (memory address locations, etc.) after the chunk header has stabilized ...
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Header Creation", "[ecs::chunk]")
    {
        const uint32_t capacity = 100;

        ChunkLayout layout;
        layout.entityCapacity = capacity;

        Chunk chunk0(0, &layout);
        Chunk chunk1(1, &layout);

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
    } LITL_END_TEST_CASE
}