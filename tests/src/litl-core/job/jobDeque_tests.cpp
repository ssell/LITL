#include <catch2/catch_test_macros.hpp>
#include "litl-core/job/jobDeque.hpp"

namespace
{
    struct JobHandleWrapper
    {
        JobHandleWrapper()
        {
            handle.job = new LITL::Core::Job();
        }

        ~JobHandleWrapper()
        {
            if (handle.job != nullptr)
            {
                delete handle.job;
                handle.job = nullptr;
            }
        }

        LITL::Core::JobHandle handle;
    };
}

TEST_CASE("Push", "[core::job::jobDeque]")
{
    LITL::Core::JobDeque deque;

    REQUIRE(deque.size() == 0);

    deque.push({});

    REQUIRE(deque.size() == 0);

    JobHandleWrapper handle0;
    JobHandleWrapper handle1;
    JobHandleWrapper handle2;

    deque.push(handle0.handle);
    REQUIRE(deque.size() == 1);

    deque.push(handle1.handle);
    REQUIRE(deque.size() == 2);

    deque.push(handle2.handle);
    REQUIRE(deque.size() == 3);
}

TEST_CASE("Pop", "[core::job::jobDeque]")
{
    // Push -> Pop should be LIFO
    // Push adds to "bottom index" and Pop removes from "bottom index"

    LITL::Core::JobDeque deque;

    JobHandleWrapper handle0;
    JobHandleWrapper handle1;
    JobHandleWrapper handle2;

    deque.push(handle0.handle);
    deque.push(handle1.handle);
    deque.push(handle2.handle);

    REQUIRE(deque.size() == 3);

    auto fetched = deque.pop();

    REQUIRE(deque.size() == 2);
    REQUIRE(fetched.has_value());
    REQUIRE((*fetched).job == handle2.handle.job);

    fetched = deque.pop();

    REQUIRE(deque.size() == 1);
    REQUIRE(fetched.has_value());
    REQUIRE((*fetched).job == handle1.handle.job);

    fetched = deque.pop();

    REQUIRE(deque.size() == 0);
    REQUIRE(fetched.has_value());
    REQUIRE((*fetched).job == handle0.handle.job);

    fetched = deque.pop();

    REQUIRE(deque.size() == 0);
    REQUIRE(!fetched.has_value());

    fetched = deque.pop();

    REQUIRE(deque.size() == 0);
    REQUIRE(!fetched.has_value());
}

TEST_CASE("Steal", "[core::job::jobDeque]")
{
    // Push -> Steal should be FIFO
    // Push adds to "bottom index" and Pop removes from "top index"

    LITL::Core::JobDeque deque;

    JobHandleWrapper handle0;
    JobHandleWrapper handle1;
    JobHandleWrapper handle2;

    deque.push(handle0.handle);
    deque.push(handle1.handle);
    deque.push(handle2.handle);

    REQUIRE(deque.size() == 3);

    auto fetched = deque.steal();

    REQUIRE(deque.size() == 2);
    REQUIRE(fetched.has_value());
    REQUIRE((*fetched).job == handle0.handle.job);

    fetched = deque.steal();

    REQUIRE(deque.size() == 1);
    REQUIRE(fetched.has_value());
    REQUIRE((*fetched).job == handle1.handle.job);

    fetched = deque.steal();

    REQUIRE(deque.size() == 0);
    REQUIRE(fetched.has_value());
    REQUIRE((*fetched).job == handle2.handle.job);

    fetched = deque.steal();

    REQUIRE(deque.size() == 0);
    REQUIRE(!fetched.has_value());

    fetched = deque.steal();

    REQUIRE(deque.size() == 0);
    REQUIRE(!fetched.has_value());
}

TEST_CASE("Big Deque", "[core::job::jobDeque]")
{
    // Add enough jobs to require multiple resizings.

    LITL::Core::JobDeque deque;

    REQUIRE(deque.capacity() == LITL::Core::JobDeque::DefaultCapacity);

    JobHandleWrapper handle;

    for (auto i = 0; i < deque.capacity(); ++i)
    {
        deque.push(handle.handle);
    }

    REQUIRE(deque.size() == LITL::Core::JobDeque::DefaultCapacity);
    REQUIRE(deque.capacity() == LITL::Core::JobDeque::DefaultCapacity);

    deque.push(handle.handle);

    REQUIRE(deque.size() == LITL::Core::JobDeque::DefaultCapacity + 1);
    REQUIRE(deque.capacity() == LITL::Core::JobDeque::DefaultCapacity * 2);

    for (auto i = deque.size(); i < deque.capacity(); ++i)
    {
        deque.push(handle.handle);
    }

    REQUIRE(deque.size() == LITL::Core::JobDeque::DefaultCapacity * 2);
    REQUIRE(deque.capacity() == LITL::Core::JobDeque::DefaultCapacity * 2);

    deque.push(handle.handle);

    REQUIRE(deque.size() == LITL::Core::JobDeque::DefaultCapacity * 2 + 1);
    REQUIRE(deque.capacity() == LITL::Core::JobDeque::DefaultCapacity * 4);

    bool anyNull = false;
    
    while (deque.size() > 0 && !anyNull)
    {
        auto handle = deque.pop();

        if (handle.value().job == nullptr)
        {
            anyNull = true;
        }
    }
    
    // If this fails, then job pointers were lost during an internal buffer resize.
    REQUIRE(anyNull == false);
}