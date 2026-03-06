#include <catch2/catch_test_macros.hpp>
#include "litl-core/work/workDeque.hpp"

TEST_CASE("Push", "[core::work]")
{
    LITL::Core::WorkDeque deque;

    REQUIRE(deque.size() == 0);

    deque.push(nullptr);

    REQUIRE(deque.size() == 0);

    LITL::Core::Job job0;
    LITL::Core::Job job1;
    LITL::Core::Job job2;

    deque.push(&job0);
    REQUIRE(deque.size() == 1);

    deque.push(&job1);
    REQUIRE(deque.size() == 2);

    deque.push(&job2);
    REQUIRE(deque.size() == 3);
}

TEST_CASE("Pop", "[core::work]")
{
    // Push -> Pop should be LIFO
    // Push adds to "bottom index" and Pop removes from "bottom index"

    LITL::Core::WorkDeque deque;

    LITL::Core::Job job0;
    LITL::Core::Job job1;
    LITL::Core::Job job2;

    deque.push(&job0);
    deque.push(&job1);
    deque.push(&job2);

    REQUIRE(deque.size() == 3);

    auto fetched = deque.pop();

    REQUIRE(deque.size() == 2);
    REQUIRE(fetched.has_value());
    REQUIRE((*fetched) == (&job2));

    fetched = deque.pop();

    REQUIRE(deque.size() == 1);
    REQUIRE(fetched.has_value());
    REQUIRE((*fetched) == (&job1));

    fetched = deque.pop();

    REQUIRE(deque.size() == 0);
    REQUIRE(fetched.has_value());
    REQUIRE((*fetched) == (&job0));

    fetched = deque.pop();

    REQUIRE(deque.size() == 0);
    REQUIRE(!fetched.has_value());

    fetched = deque.pop();

    REQUIRE(deque.size() == 0);
    REQUIRE(!fetched.has_value());
}

TEST_CASE("Steal", "[core::work]")
{
    // Push -> Steal should be FIFO
    // Push adds to "bottom index" and Pop removes from "top index"

    LITL::Core::WorkDeque deque;

    LITL::Core::Job job0;
    LITL::Core::Job job1;
    LITL::Core::Job job2;

    deque.push(&job0);
    deque.push(&job1);
    deque.push(&job2);

    REQUIRE(deque.size() == 3);

    auto fetched = deque.steal();

    REQUIRE(deque.size() == 2);
    REQUIRE(fetched.has_value());
    REQUIRE((*fetched) == (&job0));

    fetched = deque.steal();

    REQUIRE(deque.size() == 1);
    REQUIRE(fetched.has_value());
    REQUIRE((*fetched) == (&job1));

    fetched = deque.steal();

    REQUIRE(deque.size() == 0);
    REQUIRE(fetched.has_value());
    REQUIRE((*fetched) == (&job2));

    fetched = deque.steal();

    REQUIRE(deque.size() == 0);
    REQUIRE(!fetched.has_value());

    fetched = deque.steal();

    REQUIRE(deque.size() == 0);
    REQUIRE(!fetched.has_value());
}

TEST_CASE("Big Deque", "[core::work]")
{
    // Add enough jobs to require multiple resizings.

    LITL::Core::WorkDeque deque;

    REQUIRE(deque.capacity() == LITL::Core::WorkDeque::DefaultCapacity);

    LITL::Core::Job job;

    for (auto i = 0; i < deque.capacity(); ++i)
    {
        deque.push(&job);
    }

    REQUIRE(deque.size() == LITL::Core::WorkDeque::DefaultCapacity);
    REQUIRE(deque.capacity() == LITL::Core::WorkDeque::DefaultCapacity);

    deque.push(&job);

    REQUIRE(deque.size() == LITL::Core::WorkDeque::DefaultCapacity + 1);
    REQUIRE(deque.capacity() == LITL::Core::WorkDeque::DefaultCapacity * 2);

    for (auto i = deque.size(); i < deque.capacity(); ++i)
    {
        deque.push(&job);
    }

    REQUIRE(deque.size() == LITL::Core::WorkDeque::DefaultCapacity * 2);
    REQUIRE(deque.capacity() == LITL::Core::WorkDeque::DefaultCapacity * 2);

    deque.push(&job);

    REQUIRE(deque.size() == LITL::Core::WorkDeque::DefaultCapacity * 2 + 1);
    REQUIRE(deque.capacity() == LITL::Core::WorkDeque::DefaultCapacity * 4);
}