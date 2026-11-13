#include "tests.hpp"
#include "litl-core/layerRangeAllocator.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("Construction", "[engine::render::layerRangeAllocator]")
    {
        LayerRangeAllocator allocator{ 16u };

        REQUIRE(allocator.capacity() == 16u);
        REQUIRE(allocator.freeLayers() == 16u);

        LayerRangeAllocator empty{};

        REQUIRE(empty.capacity() == 0u);
        REQUIRE(empty.freeLayers() == 0u);
        REQUIRE(empty.allocate(1u).has_value() == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Allocate Zero", "[engine::render::layerRangeAllocator]")
    {
        LayerRangeAllocator allocator{ 16u };

        REQUIRE(allocator.allocate(0u).has_value() == false);
        REQUIRE(allocator.freeLayers() == 16u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Sequential Allocation", "[engine::render::layerRangeAllocator]")
    {
        LayerRangeAllocator allocator{ 16u };

        const auto a = allocator.allocate(4u);
        const auto b = allocator.allocate(4u);
        const auto c = allocator.allocate(2u);

        REQUIRE(a.has_value() == true);
        REQUIRE(b.has_value() == true);
        REQUIRE(c.has_value() == true);

        REQUIRE(a->first == 0u);
        REQUIRE(a->count == 4u);
        REQUIRE(b->first == 4u);
        REQUIRE(b->count == 4u);
        REQUIRE(c->first == 8u);
        REQUIRE(c->count == 2u);

        REQUIRE(allocator.freeLayers() == 6u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Exhaustion", "[engine::render::layerRangeAllocator]")
    {
        LayerRangeAllocator allocator{ 16u };

        const auto all = allocator.allocate(16u);

        REQUIRE(all.has_value() == true);
        REQUIRE(all->first == 0u);
        REQUIRE(all->count == 16u);
        REQUIRE(allocator.freeLayers() == 0u);
        REQUIRE(allocator.allocate(1u).has_value() == false);

        // Too large from the start
        LayerRangeAllocator small{ 4u };

        REQUIRE(small.allocate(5u).has_value() == false);
        REQUIRE(small.freeLayers() == 4u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Free And Reuse", "[engine::render::layerRangeAllocator]")
    {
        LayerRangeAllocator allocator{ 16u };

        const auto a = allocator.allocate(4u);
        const auto b = allocator.allocate(4u);

        allocator.free(*a);

        REQUIRE(allocator.freeLayers() == 12u);

        // First fit should hand back the hole at layer 0
        const auto c = allocator.allocate(4u);

        REQUIRE(c.has_value() == true);
        REQUIRE(c->first == 0u);

        allocator.free(*b);
        allocator.free(*c);

        REQUIRE(allocator.freeLayers() == 16u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Merge Previous", "[engine::render::layerRangeAllocator]")
    {
        LayerRangeAllocator allocator{ 12u };

        const auto a = allocator.allocate(4u);
        const auto b = allocator.allocate(4u);
        const auto c = allocator.allocate(4u);

        allocator.free(*a);
        allocator.free(*b);         // touches a's range on its left

        REQUIRE(allocator.freeLayers() == 8u);

        const auto merged = allocator.allocate(8u);

        REQUIRE(merged.has_value() == true);
        REQUIRE(merged->first == 0u);

        allocator.free(*merged);
        allocator.free(*c);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Merge Next", "[engine::render::layerRangeAllocator]")
    {
        LayerRangeAllocator allocator{ 12u };

        const auto a = allocator.allocate(4u);
        const auto b = allocator.allocate(4u);
        const auto c = allocator.allocate(4u);

        allocator.free(*b);
        allocator.free(*a);         // touches b's range on its right

        const auto merged = allocator.allocate(8u);

        REQUIRE(merged.has_value() == true);
        REQUIRE(merged->first == 0u);

        allocator.free(*merged);
        allocator.free(*c);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Merge Both", "[engine::render::layerRangeAllocator]")
    {
        LayerRangeAllocator allocator{ 12u };

        const auto a = allocator.allocate(4u);
        const auto b = allocator.allocate(4u);
        const auto c = allocator.allocate(4u);

        allocator.free(*a);
        allocator.free(*c);

        // Two separate holes; 8 contiguous layers are not available
        REQUIRE(allocator.freeLayers() == 8u);
        REQUIRE(allocator.allocate(8u).has_value() == false);

        allocator.free(*b);         // bridges a and c

        const auto all = allocator.allocate(12u);

        REQUIRE(all.has_value() == true);
        REQUIRE(all->first == 0u);
        REQUIRE(all->count == 12u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Free Order Independence", "[engine::render::layerRangeAllocator]")
    {
        // Every permutation of freeing three ranges must fully coalesce
        const uint32_t orders[6][3] = {
            { 0, 1, 2 }, { 0, 2, 1 }, { 1, 0, 2 },
            { 1, 2, 0 }, { 2, 0, 1 }, { 2, 1, 0 }
        };

        for (const auto& order : orders)
        {
            LayerRangeAllocator allocator{ 9u };

            const LayerRange ranges[3] = {
                *allocator.allocate(3u),
                *allocator.allocate(3u),
                *allocator.allocate(3u)
            };

            for (const uint32_t index : order)
            {
                allocator.free(ranges[index]);
            }

            REQUIRE(allocator.freeLayers() == 9u);

            const auto all = allocator.allocate(9u);

            REQUIRE(all.has_value() == true);
            REQUIRE(all->first == 0u);
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Fragmentation", "[engine::render::layerRangeAllocator]")
    {
        LayerRangeAllocator allocator{ 16u };

        const auto a = allocator.allocate(4u);
        const auto b = allocator.allocate(4u);
        const auto c = allocator.allocate(4u);
        const auto d = allocator.allocate(4u);

        allocator.free(*a);
        allocator.free(*c);

        REQUIRE(allocator.freeLayers() == 8u);
        REQUIRE(allocator.allocate(8u).has_value() == false);   // holes are not contiguous

        const auto e = allocator.allocate(4u);

        REQUIRE(e.has_value() == true);
        REQUIRE(e->first == 0u);                                // first fit

        const auto f = allocator.allocate(4u);

        REQUIRE(f.has_value() == true);
        REQUIRE(f->first == 8u);

        allocator.free(*b);
        allocator.free(*d);
        allocator.free(*e);
        allocator.free(*f);

        REQUIRE(allocator.freeLayers() == 16u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Partial Carve", "[engine::render::layerRangeAllocator]")
    {
        LayerRangeAllocator allocator{ 8u };

        const auto a = allocator.allocate(6u);

        allocator.free(*a);

        // Allocating less than a free range leaves the remainder usable
        const auto b = allocator.allocate(2u);
        const auto c = allocator.allocate(6u);

        REQUIRE(b->first == 0u);
        REQUIRE(c.has_value() == true);
        REQUIRE(c->first == 2u);
        REQUIRE(allocator.freeLayers() == 0u);
    } LITL_END_TEST_CASE
}