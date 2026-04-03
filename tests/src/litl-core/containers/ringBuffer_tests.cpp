#include "tests.hpp"
#include "litl-core/containers/ringBuffer.hpp"

namespace LITL::Core::Tests
{
    LITL_TEST_CASE("Basic", "[core::containers::ringBuffer]")
    {
        RingBuffer<uint32_t, 8> buffer{};

        REQUIRE(buffer.size() == 0);
        REQUIRE(buffer.capacity() == 8);
        REQUIRE(buffer.empty() == true);
        REQUIRE(buffer.full() == false);

        buffer.add(1);
        buffer.add(2);
        buffer.add(4);
        buffer.add(8);

        REQUIRE(buffer.size() == 4);
        REQUIRE(buffer.capacity() == 8);
        REQUIRE(buffer.empty() == false);
        REQUIRE(buffer.full() == false);

        buffer.add(1);
        buffer.add(2);
        buffer.add(4);
        buffer.add(8);

        REQUIRE(buffer.size() == 8);
        REQUIRE(buffer.capacity() == 8);
        REQUIRE(buffer.empty() == false);
        REQUIRE(buffer.full() == true);

        buffer.add(8);
        buffer.add(4);
        buffer.add(2);
        buffer.add(1);

        REQUIRE(buffer.size() == 8);
        REQUIRE(buffer.capacity() == 8);
        REQUIRE(buffer.empty() == false);
        REQUIRE(buffer.full() == true);

        REQUIRE(buffer[0] == 1);
        REQUIRE(buffer[1] == 2);
        REQUIRE(buffer[2] == 4);
        REQUIRE(buffer[3] == 8);
        REQUIRE(buffer[4] == 8);
        REQUIRE(buffer[5] == 4);
        REQUIRE(buffer[6] == 2);
        REQUIRE(buffer[7] == 1);

    } END_LITL_TEST_CASE

    LITL_TEST_CASE("List", "[core::containers::ringBuffer]")
    {
        RingBuffer<uint32_t, 8> buffer{ { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } };

        REQUIRE(buffer.size() == 8);
        REQUIRE(buffer.capacity() == 8);
        REQUIRE(buffer.empty() == false);
        REQUIRE(buffer.full() == true);

        REQUIRE(buffer[0] == 3);
        REQUIRE(buffer[7] == 10);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Iterator", "[core::containers::ringBuffer]")
    {
        RingBuffer<uint32_t, 8> buffer{ { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } };

        uint32_t expected = 3;

        for (auto& value : buffer)
        {
            REQUIRE(value == expected++);
        }

    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Sum", "[core::containers::ringBuffer]")
    {
        RingBuffer<uint32_t, 8> buffer{ { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } };

        uint32_t expected = (3 + 4 + 5 + 6 + 7 + 8 + 9 + 10);
        uint32_t sum = buffer.sum();

        REQUIRE(sum == expected);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Min", "[core::containers::ringBuffer]")
    {
        RingBuffer<uint32_t, 8> buffer{ { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } };

        uint32_t expected = 3;
        uint32_t min = buffer.min();

        REQUIRE(min == expected);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Max", "[core::containers::ringBuffer]")
    {
        RingBuffer<uint32_t, 8> buffer{ { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } };

        uint32_t expected = 10;
        uint32_t max = buffer.max();

        REQUIRE(max == expected);
    } END_LITL_TEST_CASE
}