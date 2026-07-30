#include "tests.hpp"
#include <coroutine>

/**
 * I'll be honest.
 * 
 * I don't understand coroutines in C++. But I am pretty sure I want them for my implementation of multi-frame Tasks.
 * So, here is me trying to learn them.
 */

namespace litl::tests
{
    namespace
    {
        /// <summary>
        /// Support for co_yield only.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        template<typename T>
        struct Generator
        {
            struct promise_type
            {
                T value{};

                Generator get_return_object() { return Generator{ std::coroutine_handle<promise_type>::from_promise(*this) }; }
                std::suspend_always initial_suspend() noexcept { return {}; }                   // lazy start
                std::suspend_always final_suspend() noexcept { return {}; }                     // don't self-destruct
                std::suspend_always yield_value(T v) { value = std::move(v); return {}; }       // how to return the value
                void return_void() {}                                                           // invoked when the coroutine method ends (i think?)
                void unhandled_exception() {}
            };

            using coroutine_handle = std::coroutine_handle<promise_type>;

            explicit Generator(coroutine_handle handle) : handle(handle) {}
            ~Generator() { if (handle) { handle.destroy(); } }
            Generator(Generator&& other) noexcept : handle(std::exchange(other.handle, {})) {}
            Generator(Generator const&) = delete;

            bool next()
            {
                if (!handle || handle.done())
                {
                    return false;
                }

                handle.resume();

                return !handle.done();
            }

            T& value()
            {
                return handle.promise().value;
            }

            coroutine_handle handle;
        };

        Generator<uint32_t> summation(uint32_t start, uint32_t end)
        {
            uint32_t value = 0u;

            for (uint32_t i = start; i <= end; ++i)
            {
                value += i;
                co_yield value;
            }
        }
    }

    LITL_TEST_CASE("Summation", "[core::tasks]")
    {
        auto sum1to10 = summation(1, 10);

        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 1u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 3u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 6u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 10u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 15u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 21u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 28u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 36u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 45u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 55u);
        REQUIRE(sum1to10.next() == false);
    } LITL_END_TEST_CASE
}