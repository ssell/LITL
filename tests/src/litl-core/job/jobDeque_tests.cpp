#include "tests.hpp"
#include "litl-core/job/jobDeque.hpp"

namespace LITL::Core::Tests
{
    namespace
    {
        struct JobHandleWrapper
        {
            JobHandleWrapper()
            {
                static uint32_t JobIndex = 1;
                handle = { 0, JobIndex++ };
            }

            JobHandle handle;
        };
    }

    LITL_TEST_CASE("Push", "[core::job::jobDeque]")
    {
        JobDeque deque;

        REQUIRE(deque.size() == 0);

        deque.push({});

        REQUIRE(deque.size() == 1);

        JobHandleWrapper handle0;
        JobHandleWrapper handle1;
        JobHandleWrapper handle2;

        deque.push(handle0.handle);
        REQUIRE(deque.size() == 2);

        deque.push(handle1.handle);
        REQUIRE(deque.size() == 3);

        deque.push(handle2.handle);
        REQUIRE(deque.size() == 4);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Pop", "[core::job::jobDeque]")
    {
        // Push -> Pop should be LIFO
        // Push adds to "bottom index" and Pop removes from "bottom index"

        JobDeque deque;

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
        REQUIRE((*fetched) == handle2.handle);

        fetched = deque.pop();

        REQUIRE(deque.size() == 1);
        REQUIRE(fetched.has_value());
        REQUIRE((*fetched) == handle1.handle);

        fetched = deque.pop();

        REQUIRE(deque.size() == 0);
        REQUIRE(fetched.has_value());
        REQUIRE((*fetched) == handle0.handle);

        fetched = deque.pop();

        REQUIRE(deque.size() == 0);
        REQUIRE(!fetched.has_value());

        fetched = deque.pop();

        REQUIRE(deque.size() == 0);
        REQUIRE(!fetched.has_value());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Steal", "[core::job::jobDeque]")
    {
        // Push -> Steal should be FIFO
        // Push adds to "bottom index" and Pop removes from "top index"

        JobDeque deque;

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
        REQUIRE((*fetched) == handle0.handle);

        fetched = deque.steal();

        REQUIRE(deque.size() == 1);
        REQUIRE(fetched.has_value());
        REQUIRE((*fetched) == handle1.handle);

        fetched = deque.steal();

        REQUIRE(deque.size() == 0);
        REQUIRE(fetched.has_value());
        REQUIRE((*fetched) == handle2.handle);

        fetched = deque.steal();

        REQUIRE(deque.size() == 0);
        REQUIRE(!fetched.has_value());

        fetched = deque.steal();

        REQUIRE(deque.size() == 0);
        REQUIRE(!fetched.has_value());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Big Deque", "[core::job::jobDeque]")
    {
        // Add enough jobs to require multiple resizings.

        JobDeque deque;

        REQUIRE(deque.capacity() == JobDeque::DefaultCapacity);

        JobHandleWrapper handle;

        for (auto i = 0; i < deque.capacity(); ++i)
        {
            deque.push(handle.handle);
        }

        REQUIRE(deque.size() == JobDeque::DefaultCapacity);
        REQUIRE(deque.capacity() == JobDeque::DefaultCapacity);

        deque.push(handle.handle);

        REQUIRE(deque.size() == JobDeque::DefaultCapacity + 1);
        REQUIRE(deque.capacity() == JobDeque::DefaultCapacity * 2);

        for (auto i = deque.size(); i < deque.capacity(); ++i)
        {
            deque.push(handle.handle);
        }

        REQUIRE(deque.size() == JobDeque::DefaultCapacity * 2);
        REQUIRE(deque.capacity() == JobDeque::DefaultCapacity * 2);

        deque.push(handle.handle);

        REQUIRE(deque.size() == JobDeque::DefaultCapacity * 2 + 1);
        REQUIRE(deque.capacity() == JobDeque::DefaultCapacity * 4);

        bool anyNull = false;

        while (deque.size() > 0 && !anyNull)
        {
            auto handle = deque.pop();

            if (handle.has_value() && (*handle).isNull())
            {
                anyNull = true;
            }
        }

        // If this fails, then job pointers were lost during an internal buffer resize.
        REQUIRE(anyNull == false);
    } END_LITL_TEST_CASE
}