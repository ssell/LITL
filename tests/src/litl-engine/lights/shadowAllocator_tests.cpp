#include "tests.hpp"
#include "litl-engine/lights/shadowAllocator.hpp"

namespace litl::tests
{
    namespace
    {
        constexpr uint32_t TestFramesInFlight = 2u;
        constexpr uint32_t TestCascades = 4u;

        [[nodiscard]] ShadowAllocator makeAllocator(uint32_t directionalLayers, uint32_t spotLayers = 0u, uint32_t pointCubes = 0u)
        {
            ShadowAllocator allocator{};
            allocator.setup(ShadowPoolCapacities{
                .directionalLayers = directionalLayers,
                .spotLayers = spotLayers,
                .pointCubes = pointCubes
                }, TestFramesInFlight);

            return allocator;
        }

        [[nodiscard]] ShadowKey directionalKey(uint32_t entityIndex, uint32_t cameraSlot, uint32_t version = 0u)
        {
            return ShadowKey{ .light = Entity{.index = entityIndex, .version = version }, .cameraSlot = cameraSlot };
        }

        [[nodiscard]] bool overlaps(LayerRange const& a, LayerRange const& b)
        {
            return (a.first < b.end()) && (b.first < a.end());
        }
    }

    LITL_TEST_CASE("Init", "[engine::render::shadowAllocator]")
    {
        auto allocator = makeAllocator(16u, 8u, 2u);

        REQUIRE(allocator.freeUnits(ShadowCasterType::Directional) == 16u);
        REQUIRE(allocator.freeUnits(ShadowCasterType::Spot) == 8u);
        REQUIRE(allocator.freeUnits(ShadowCasterType::Point) == 2u);
        REQUIRE(allocator.activeAllocationCount() == 0u);
        REQUIRE(allocator.pendingRetireCount() == 0u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Acquire New", "[engine::render::shadowAllocator]")
    {
        auto allocator = makeAllocator(16u);

        allocator.beginFrame(1u);
        const auto result = allocator.acquire(directionalKey(1u, 0u), ShadowCasterType::Directional, TestCascades, 1u);

        REQUIRE(result.has_value() == true);
        REQUIRE(result->isNew == true);
        REQUIRE(result->allocation.type == ShadowCasterType::Directional);
        REQUIRE(result->allocation.range.first == 0u);
        REQUIRE(result->allocation.range.count == TestCascades);
        REQUIRE(allocator.activeAllocationCount() == 1u);
        REQUIRE(allocator.freeUnits(ShadowCasterType::Directional) == 12u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Acquire Stable Across Frames", "[engine::render::shadowAllocator]")
    {
        auto allocator = makeAllocator(16u);
        const auto key = directionalKey(1u, 0u);

        allocator.beginFrame(1u);
        const auto first = allocator.acquire(key, ShadowCasterType::Directional, TestCascades, 1u);
        allocator.releaseUnused(1u);

        for (uint64_t frame = 2u; frame < 10u; ++frame)
        {
            allocator.beginFrame(frame);
            const auto again = allocator.acquire(key, ShadowCasterType::Directional, TestCascades, frame);
            allocator.releaseUnused(frame);

            REQUIRE(again.has_value() == true);
            REQUIRE(again->isNew == false);
            REQUIRE(again->allocation.range.first == first->allocation.range.first);
            REQUIRE(again->allocation.range.count == first->allocation.range.count);
            REQUIRE(again->allocation.version == first->allocation.version);
        }

        REQUIRE(allocator.activeAllocationCount() == 1u);
        REQUIRE(allocator.pendingRetireCount() == 0u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Acquire Same Key Twice In Frame", "[engine::render::shadowAllocator]")
    {
        auto allocator = makeAllocator(16u);
        const auto key = directionalKey(1u, 0u);

        allocator.beginFrame(1u);
        const auto a = allocator.acquire(key, ShadowCasterType::Directional, TestCascades, 1u);
        const auto b = allocator.acquire(key, ShadowCasterType::Directional, TestCascades, 1u);

        REQUIRE(a->isNew == true);
        REQUIRE(b->isNew == false);
        REQUIRE(a->allocation.range.first == b->allocation.range.first);
        REQUIRE(allocator.activeAllocationCount() == 1u);
        REQUIRE(allocator.freeUnits(ShadowCasterType::Directional) == 12u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Distinct Camera Slots", "[engine::render::shadowAllocator]")
    {
        auto allocator = makeAllocator(16u);

        allocator.beginFrame(1u);
        const auto a = allocator.acquire(directionalKey(1u, 0u), ShadowCasterType::Directional, TestCascades, 1u);
        const auto b = allocator.acquire(directionalKey(1u, 1u), ShadowCasterType::Directional, TestCascades, 1u);

        REQUIRE(a.has_value() == true);
        REQUIRE(b.has_value() == true);
        REQUIRE(overlaps(a->allocation.range, b->allocation.range) == false);
        REQUIRE(a->allocation.version != b->allocation.version);
        REQUIRE(allocator.activeAllocationCount() == 2u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Distinct Entity Version", "[engine::render::shadowAllocator]")
    {
        auto allocator = makeAllocator(16u);

        // Same index, new version = a different (recycled) entity. Must not alias.
        allocator.beginFrame(1u);
        const auto oldLight = allocator.acquire(directionalKey(1u, 0u, 0u), ShadowCasterType::Directional, TestCascades, 1u);
        const auto newLight = allocator.acquire(directionalKey(1u, 0u, 1u), ShadowCasterType::Directional, TestCascades, 1u);

        REQUIRE(newLight->isNew == true);
        REQUIRE(overlaps(oldLight->allocation.range, newLight->allocation.range) == false);
        REQUIRE(allocator.activeAllocationCount() == 2u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Release Unused Retires", "[engine::render::shadowAllocator]")
    {
        auto allocator = makeAllocator(16u);
        const auto kept = directionalKey(1u, 0u);
        const auto dropped = directionalKey(1u, 1u);

        allocator.beginFrame(1u);
        (void)allocator.acquire(kept, ShadowCasterType::Directional, TestCascades, 1u);
        (void)allocator.acquire(dropped, ShadowCasterType::Directional, TestCascades, 1u);
        allocator.releaseUnused(1u);

        REQUIRE(allocator.activeAllocationCount() == 2u);

        // Frame 2: camera slot 1 no longer wants shadows
        allocator.beginFrame(2u);
        (void)allocator.acquire(kept, ShadowCasterType::Directional, TestCascades, 2u);
        allocator.releaseUnused(2u);

        REQUIRE(allocator.activeAllocationCount() == 1u);
        REQUIRE(allocator.pendingRetireCount() == 1u);
        REQUIRE(allocator.freeUnits(ShadowCasterType::Directional) == 8u);     // not returned yet
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Retire Delay", "[engine::render::shadowAllocator]")
    {
        auto allocator = makeAllocator(16u);
        const auto key = directionalKey(1u, 0u);

        allocator.beginFrame(1u);
        (void)allocator.acquire(key, ShadowCasterType::Directional, TestCascades, 1u);
        allocator.releaseUnused(1u);

        // Frame 2: not acquired -> retired at frame 2
        allocator.beginFrame(2u);
        allocator.releaseUnused(2u);

        REQUIRE(allocator.pendingRetireCount() == 1u);
        REQUIRE(allocator.freeUnits(ShadowCasterType::Directional) == 12u);

        // Frame 3: only 1 frame elapsed, still in flight
        allocator.beginFrame(3u);
        allocator.releaseUnused(3u);

        REQUIRE(allocator.pendingRetireCount() == 1u);
        REQUIRE(allocator.freeUnits(ShadowCasterType::Directional) == 12u);

        // Frame 4: framesInFlight (2) elapsed, safe to reuse
        allocator.beginFrame(4u);

        REQUIRE(allocator.pendingRetireCount() == 0u);
        REQUIRE(allocator.freeUnits(ShadowCasterType::Directional) == 16u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Retired Range Not Reused Early", "[engine::render::shadowAllocator]")
    {
        auto allocator = makeAllocator(8u);     // room for exactly two allocations

        allocator.beginFrame(1u);
        const auto a = allocator.acquire(directionalKey(1u, 0u), ShadowCasterType::Directional, TestCascades, 1u);
        (void)allocator.acquire(directionalKey(1u, 1u), ShadowCasterType::Directional, TestCascades, 1u);
        allocator.releaseUnused(1u);

        // Frame 2: drop slot 0, and a new camera immediately wants shadows
        allocator.beginFrame(2u);
        (void)allocator.acquire(directionalKey(1u, 1u), ShadowCasterType::Directional, TestCascades, 2u);
        const auto early = allocator.acquire(directionalKey(1u, 2u), ShadowCasterType::Directional, TestCascades, 2u);
        allocator.releaseUnused(2u);

        // Slot 0's range was retired this frame (after the acquire), and the pool is otherwise full
        REQUIRE(early.has_value() == false);

        // Frame 4: retired range is back and can go to the new camera
        allocator.beginFrame(3u);
        (void)allocator.acquire(directionalKey(1u, 1u), ShadowCasterType::Directional, TestCascades, 3u);
        allocator.releaseUnused(3u);

        allocator.beginFrame(4u);
        (void)allocator.acquire(directionalKey(1u, 1u), ShadowCasterType::Directional, TestCascades, 4u);
        const auto late = allocator.acquire(directionalKey(1u, 2u), ShadowCasterType::Directional, TestCascades, 4u);

        REQUIRE(late.has_value() == true);
        REQUIRE(late->isNew == true);
        REQUIRE(late->allocation.range.first == a->allocation.range.first);
        REQUIRE(late->allocation.version != a->allocation.version);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Pool Full", "[engine::render::shadowAllocator]")
    {
        auto allocator = makeAllocator(8u);

        allocator.beginFrame(1u);
        const auto a = allocator.acquire(directionalKey(1u, 0u), ShadowCasterType::Directional, TestCascades, 1u);
        const auto b = allocator.acquire(directionalKey(1u, 1u), ShadowCasterType::Directional, TestCascades, 1u);
        const auto c = allocator.acquire(directionalKey(1u, 2u), ShadowCasterType::Directional, TestCascades, 1u);

        REQUIRE(a.has_value() == true);
        REQUIRE(b.has_value() == true);
        REQUIRE(c.has_value() == false);
        REQUIRE(allocator.activeAllocationCount() == 2u);       // failed acquire leaves no entry
        REQUIRE(allocator.freeUnits(ShadowCasterType::Directional) == 0u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Count Change Reallocates", "[engine::render::shadowAllocator]")
    {
        auto allocator = makeAllocator(16u);
        const auto key = directionalKey(1u, 0u);

        allocator.beginFrame(1u);
        const auto four = allocator.acquire(key, ShadowCasterType::Directional, 4u, 1u);
        allocator.releaseUnused(1u);

        allocator.beginFrame(2u);
        const auto two = allocator.acquire(key, ShadowCasterType::Directional, 2u, 2u);
        allocator.releaseUnused(2u);

        REQUIRE(two.has_value() == true);
        REQUIRE(two->isNew == true);
        REQUIRE(two->allocation.range.count == 2u);
        REQUIRE(overlaps(two->allocation.range, four->allocation.range) == false);     // old range still retiring
        REQUIRE(allocator.activeAllocationCount() == 1u);
        REQUIRE(allocator.pendingRetireCount() == 1u);

        allocator.beginFrame(4u);

        REQUIRE(allocator.freeUnits(ShadowCasterType::Directional) == 14u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Explicit Release", "[engine::render::shadowAllocator]")
    {
        auto allocator = makeAllocator(16u);
        const auto key = directionalKey(1u, 0u);

        allocator.beginFrame(1u);
        (void)allocator.acquire(key, ShadowCasterType::Directional, TestCascades, 1u);
        allocator.release(key, 1u);

        REQUIRE(allocator.activeAllocationCount() == 0u);
        REQUIRE(allocator.pendingRetireCount() == 1u);

        // Releasing an unknown key is a no-op
        allocator.release(directionalKey(99u, 0u), 1u);

        REQUIRE(allocator.pendingRetireCount() == 1u);

        allocator.beginFrame(3u);

        REQUIRE(allocator.freeUnits(ShadowCasterType::Directional) == 16u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Pools Are Independent", "[engine::render::shadowAllocator]")
    {
        auto allocator = makeAllocator(4u, 4u, 1u);
        const ShadowKey spotKey{ .light = Entity{.index = 2u, .version = 0u } };
        const ShadowKey pointKey{ .light = Entity{.index = 3u, .version = 0u } };

        allocator.beginFrame(1u);
        const auto dir = allocator.acquire(directionalKey(1u, 0u), ShadowCasterType::Directional, 4u, 1u);
        const auto spot = allocator.acquire(spotKey, ShadowCasterType::Spot, 1u, 1u);
        const auto point = allocator.acquire(pointKey, ShadowCasterType::Point, 1u, 1u);

        REQUIRE(dir.has_value() == true);
        REQUIRE(spot.has_value() == true);
        REQUIRE(point.has_value() == true);

        // Each pool starts at 0; ranges index different textures
        REQUIRE(dir->allocation.range.first == 0u);
        REQUIRE(spot->allocation.range.first == 0u);
        REQUIRE(point->allocation.range.first == 0u);

        REQUIRE(allocator.freeUnits(ShadowCasterType::Directional) == 0u);
        REQUIRE(allocator.freeUnits(ShadowCasterType::Spot) == 3u);
        REQUIRE(allocator.freeUnits(ShadowCasterType::Point) == 0u);

        // Point pool is full, but the spot pool is unaffected
        REQUIRE(allocator.acquire(ShadowKey{ .light = Entity{.index = 4u } }, ShadowCasterType::Point, 1u, 1u).has_value() == false);
        REQUIRE(allocator.acquire(ShadowKey{ .light = Entity{.index = 5u } }, ShadowCasterType::Spot, 1u, 1u).has_value() == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Setup Resets State", "[engine::render::shadowAllocator]")
    {
        auto allocator = makeAllocator(16u);

        allocator.beginFrame(1u);
        (void)allocator.acquire(directionalKey(1u, 0u), ShadowCasterType::Directional, TestCascades, 1u);
        allocator.release(directionalKey(1u, 0u), 1u);

        allocator.setup(ShadowPoolCapacities{ .directionalLayers = 8u }, TestFramesInFlight);

        REQUIRE(allocator.activeAllocationCount() == 0u);
        REQUIRE(allocator.pendingRetireCount() == 0u);
        REQUIRE(allocator.freeUnits(ShadowCasterType::Directional) == 8u);
    } LITL_END_TEST_CASE
}