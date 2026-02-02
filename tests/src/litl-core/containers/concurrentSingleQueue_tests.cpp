#include <catch2/catch_test_macros.hpp>
#include <cstdint>

#include "litl-core/containers/concurrentSingleQueue.hpp"

TEST_CASE("Push-Pop Single-Threaded", "[core::containers::concurrentSingleQueue]")
{
    LITL::Core::Containers::ConcurrentSingleQueue<uint32_t> queue{ 5 };

    REQUIRE(queue.size() == 0);
    REQUIRE(queue.capacity() == 5);
    REQUIRE(queue.push(1) == true);
    REQUIRE(queue.push(2) == true);
    REQUIRE(queue.push(3) == true);
    REQUIRE(queue.push(4) == true);
    REQUIRE(queue.push(5) == true);
    REQUIRE(queue.push(6) == false);
    REQUIRE(queue.empty() == false);
    REQUIRE(queue.full() == true);
    REQUIRE(queue.size() == 5);
    REQUIRE(queue.pop() == 1);
    REQUIRE(queue.full() == false);
    REQUIRE(queue.pop() == 2);
    REQUIRE(queue.pop() == 3);
    REQUIRE(queue.pop() == 4);
    REQUIRE(queue.pop() == 5);
    REQUIRE(queue.pop() == std::nullopt);
    REQUIRE(queue.size() == 0);
    REQUIRE(queue.empty() == true);
}

TEST_CASE("Push-Peek Single-Threaded", "[core::containers::concurrentSingleQueue]")
{
    LITL::Core::Containers::ConcurrentSingleQueue<uint32_t> queue{ 5 };

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

TEST_CASE("Push-Clear Single-Threaded", "[core::containers::concurrentSingleQueue]")
{
    LITL::Core::Containers::ConcurrentSingleQueue<uint32_t> queue{ 50 };

    for (int i = 0; i < 50; ++i)
    {
        queue.push(i);
    }

    REQUIRE(queue.size() == 50);
    REQUIRE(queue.peek() == 0);     // queue is FIFO

    queue.clear();

    REQUIRE(queue.size() == 0);
    REQUIRE(queue.peek() == std::nullopt);
}