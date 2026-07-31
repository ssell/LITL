#include "tests.hpp"
#include "litl-core/moveOnlyFunc.hpp"
#include <deque>

// Global allocation counter. Lets us assert that the construction did not allocate.
static std::size_t g_allocations = 0ull;
void* operator new(std::size_t n) { ++g_allocations; return std::malloc(n); }
void  operator delete(void* p) noexcept { std::free(p); }
void  operator delete(void* p, std::size_t) noexcept { std::free(p); }

namespace litl::tests
{
    namespace
    {

        struct MoveOnlyFuncProbe
        {
            static inline uint32_t live = 0u;
            static inline uint32_t ctors = 0u;
            static inline uint32_t dtors = 0u;

            int32_t id = -1;

            explicit MoveOnlyFuncProbe(int32_t i = 0u)
                : id(i)
            {
                ++ctors;
                ++live;
            }

            MoveOnlyFuncProbe(MoveOnlyFuncProbe const& other)
                : id(other.id)
            {
                ++ctors;
                ++live;
            }

            MoveOnlyFuncProbe(MoveOnlyFuncProbe&& other)
                : id(other.id)
            {
                other.id = -1;
                ++ctors;
                ++live;
            }

            ~MoveOnlyFuncProbe()
            {
                ++dtors;
                --live;
            }

            static void reset()
            {
                live = 0u;
                ctors = 0u;
                dtors = 0u;
            }
        };
    }


    LITL_TEST_CASE("Empty State", "[core::MoveOnlyFunc]")
    {
        MoveOnlyFunc<int()> func;

        REQUIRE_FALSE(func);
        REQUIRE_FALSE(MoveOnlyFunc<int()>{ nullptr });
        
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Invoke and Returns", "[core::MoveOnlyFunc]")
    {
        MoveOnlyFunc<int(int)> func = [](int x) -> int { return x * 2; };

        REQUIRE(func);
        REQUIRE(func(21) == 42);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Small Callable is Inlined", "[core::MoveOnlyFunc]")
    {
        struct SmallState
        {
            uint32_t value = 0u;
        };

        // if this fails then we cant expect it to be moved to the inline buffer
        static_assert(sizeof(SmallState) <= MoveOnlyFunc<void()>::BufferSize);

        auto state = SmallState{};
        auto allocsBefore = g_allocations;

        MoveOnlyFunc<void()> func = [&state]() -> void { state.value += 1u; };

        REQUIRE((g_allocations - allocsBefore) == 0);
        REQUIRE(func);
        
        REQUIRE(state.value == 0u);
        func();
        REQUIRE(state.value == 1u);

    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Large Callable is Heap Allocated", "[core::MoveOnlyFunc]")
    {
        struct LargeState
        {
            std::array<std::byte, MoveOnlyFunc<void()>::BufferSize * 2u> data;
            LargeState() { data.fill(std::byte(0)); }
        };

        // if this fails then we cant expect it to be heap allocated
        static_assert(sizeof(LargeState) > MoveOnlyFunc<void()>::BufferSize);

        auto state = LargeState{}; 
        auto allocsBefore = g_allocations;

        state.data.fill(std::byte(1));
        MoveOnlyFunc<std::byte()> func = [state]() -> std::byte { return state.data[0]; };

        REQUIRE((g_allocations - allocsBefore) == 1);
        REQUIRE(func);
        REQUIRE(func() == std::byte(1));

    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Holds a Move-Only Payload", "[core::MoveOnlyFunc]")
    {
        MoveOnlyFunc<int()> func = [p = std::make_unique<int>(42)] { return *p; };

        CHECK(func() == 42);            // source has payload
        auto g = std::move(func);
        CHECK_FALSE(func);              // source was emptied
        CHECK(g() == 42);               // payload followed the move
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Lifetime Balance through Moves", "[core::MoveOnlyFunc]")
    {
        MoveOnlyFuncProbe::reset();
        
        {
            std::deque<MoveOnlyFunc<int()>> dq;

            for (int i = 0; i < 8; ++i)
            {
                dq.emplace_back([probe = MoveOnlyFuncProbe{ i }]() -> int { return probe.id; });
            }

            auto dq2 = std::move(dq);
            int sum = 0;

            for (auto& func : dq2)
            {
                sum += func();
            }

            CHECK(sum == 28);
        }

        CHECK(MoveOnlyFuncProbe::live == 0);                            // catch missing destroy
        CHECK(MoveOnlyFuncProbe::ctors > 0);
        CHECK(MoveOnlyFuncProbe::ctors == MoveOnlyFuncProbe::dtors);    // catch double destroy
    } LITL_END_TEST_CASE
}