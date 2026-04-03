#include <optional>

#include "tests.hpp"
#include "litl-core/containers/memoryArena.hpp"

namespace LITL::Core::Tests
{
    namespace
    {
        struct TestStructA
        {
            uint32_t a{ 0 };
        };

        struct TestStructB
        {
            uint32_t a{ 0 };
            uint32_t b{ 0 };
        };
    }

    LITL_TEST_CASE("Insert", "[core::containers::memoryArena]")
    {
        MemoryArena<> arena;

        TestStructA a{ 55 };
        TestStructB b{ 100, 33 };

        void* aDest = arena.insert(&a, sizeof(TestStructA), alignof(TestStructA));
        void* bDest = arena.insert<TestStructB>(b);

        TestStructA aExtracted{};
        TestStructB bExtracted{};

        REQUIRE(aExtracted.a != a.a);
        REQUIRE(bExtracted.a != b.a);
        REQUIRE(bExtracted.b != b.b);

        memcpy(&aExtracted, aDest, sizeof(TestStructA));
        memcpy(&bExtracted, bDest, sizeof(TestStructB));

        REQUIRE(aExtracted.a == a.a);
        REQUIRE(bExtracted.a == b.a);
        REQUIRE(bExtracted.b == b.b);

        auto const& state = arena.getState();

        REQUIRE(state.blocksAllocated == 1);
        REQUIRE(state.bytesAllocated() == state.blockSize);
        REQUIRE(state.blocksInUse == 1);
        REQUIRE(state.bytesInUse == state.currBlockOffset);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Emplace", "[core::containers::memoryArena]")
    {
        MemoryArena<> arena;

        auto* a = arena.emplace<TestStructA>(55);
        auto* b = arena.emplace<TestStructB>(105, 337);

        REQUIRE(a != nullptr);
        REQUIRE(b != nullptr);
        REQUIRE(a->a == 55);
        REQUIRE(b->a == 105);
        REQUIRE(b->b == 337);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Move Constructor", "[core::containers::memoryArena]")
    {
        MemoryArena<> arena0;

        auto* a = arena0.emplace<TestStructA>(55);
        auto* b = arena0.emplace<TestStructB>(105, 337);

        auto state0A = arena0.getState();

        MemoryArena<> arena1(std::move(arena0));

        auto state0B = arena0.getState();
        auto state1A = arena1.getState();

        REQUIRE(a != nullptr);
        REQUIRE(b != nullptr);
        REQUIRE(a->a == 55);
        REQUIRE(b->a == 105);
        REQUIRE(b->b == 337);

        REQUIRE(state0A.currBlockOffset == state1A.currBlockOffset);
        REQUIRE(state0B.currBlockOffset == 0);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Move Assignment", "[core::containers::memoryArena]")
    {
        MemoryArena<> arena0;

        auto* a = arena0.emplace<TestStructA>(55);
        auto* b = arena0.emplace<TestStructB>(105, 337);

        auto state0A = arena0.getState();

        MemoryArena<> arena1 = std::move(arena0);

        auto state0B = arena0.getState();
        auto state1A = arena1.getState();

        REQUIRE(a != nullptr);
        REQUIRE(b != nullptr);
        REQUIRE(a->a == 55);
        REQUIRE(b->a == 105);
        REQUIRE(b->b == 337);

        REQUIRE(state0A.currBlockOffset == state1A.currBlockOffset);
        REQUIRE(state0B.currBlockOffset == 0);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Many", "[core::containers::memoryArena]")
    {
        MemoryArena<> arena;
        TestStructA a{ 0 };

        constexpr uint32_t objectCount = 8192;
        constexpr uint32_t expectedMaxBlockCount = 32;
        constexpr uint32_t expectedMaxByteCount = 32768;

        std::optional<MemoryArenaState> statePriorToFirstReset = std::nullopt;

        for (auto i = 0; i < 10; ++i)
        {
            TestStructA first{ 0 };
            TestStructA last{ 0 };

            auto currState = arena.getState();

            REQUIRE(currState.blocksInUse == 1);
            REQUIRE(currState.bytesInUse == 0);
            REQUIRE(currState.currBlockOffset == 0);

            if (statePriorToFirstReset.has_value())
            {
                // Offsets, etc. reset but memory usage should stay the same.
                REQUIRE(currState.blocksAllocated == (*statePriorToFirstReset).blocksAllocated);
                REQUIRE(currState.bytesAllocated() == (*statePriorToFirstReset).bytesAllocated());
            }

            for (auto j = 0; j < objectCount; ++j)
            {
                a.a = j + 1;
                auto dest = arena.insert<TestStructA>(a);

                if (j == 0)
                {
                    memcpy(&first, dest, sizeof(TestStructA));
                }
                else if (j == (objectCount - 1))
                {
                    memcpy(&last, dest, sizeof(TestStructA));
                }
            }

            REQUIRE(first.a == 1);
            REQUIRE(last.a == objectCount);

            if (statePriorToFirstReset.has_value())
            {
                currState = arena.getState();

                REQUIRE(currState.blocksAllocated == (*statePriorToFirstReset).blocksAllocated);
                REQUIRE(currState.bytesAllocated() == (*statePriorToFirstReset).bytesAllocated());
                REQUIRE(currState.blocksInUse == (*statePriorToFirstReset).blocksInUse);
                REQUIRE(currState.bytesInUse == (*statePriorToFirstReset).bytesInUse);
                REQUIRE(currState.currBlockOffset == (*statePriorToFirstReset).currBlockOffset);
            }
            else
            {
                statePriorToFirstReset = arena.getState();

                REQUIRE(statePriorToFirstReset.value().blocksAllocated == expectedMaxBlockCount);
                REQUIRE(statePriorToFirstReset.value().bytesAllocated() == expectedMaxByteCount);
            }

            arena.reset();
        }

    } END_LITL_TEST_CASE
}