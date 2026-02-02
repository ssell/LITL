#include <catch2/catch_test_macros.hpp>
#include <cstdint>

#include "litl-core/containers/concurrentQueue.hpp"

TEST_CASE("Push-Pop Single-Threaded", "[core::containers]")
{
    LITL::Core::Containers::ConcurrentQueue<uint32_t> queue{};

    REQUIRE(queue.size() == 0);

    queue.push(55);

    REQUIRE(queue.size() == 1);
    REQUIRE(queue.pop() == 55);
    REQUIRE(queue.size() == 0);
}

TEST_CASE("Push-Peek Single-Threaded", "[core::containers]")
{
    LITL::Core::Containers::ConcurrentQueue<uint32_t> queue{};

    REQUIRE(queue.size() == 0);
    REQUIRE(queue.peek() == std::nullopt);

    queue.push(55);

    REQUIRE(queue.size() == 1);
    REQUIRE(queue.peek() == 55);
    REQUIRE(queue.size() == 1);
    REQUIRE(queue.pop() == 55);
    REQUIRE(queue.peek() == std::nullopt);
    REQUIRE(queue.size() == 0);
}