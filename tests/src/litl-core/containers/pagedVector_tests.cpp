#include "tests.hpp"
#include <cstdint>

#include "litl-core/containers/pagedVector.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("Push-Pop", "[core::containers::pagedVector]")
    {
        PagedVector<uint32_t, 32> vector;

        REQUIRE(vector.size() == 0);
        REQUIRE(vector.capacity() == 0);
        REQUIRE(vector.empty() == true);
        REQUIRE(vector.full() == true);

        vector.push_back(0);

        REQUIRE(vector.size() == 1);
        REQUIRE(vector.capacity() == 32);
        REQUIRE(vector.empty() == false);
        REQUIRE(vector.full() == false);

        for (auto i = 0; i < 31; ++i)
        {
            vector.push_back(i);
        }

        REQUIRE(vector.size() == 32);
        REQUIRE(vector.capacity() == 32);
        REQUIRE(vector.empty() == false);
        REQUIRE(vector.full() == true);

        vector.push_back(0);

        REQUIRE(vector.size() == 33);
        REQUIRE(vector.capacity() == 64);
        REQUIRE(vector.empty() == false);
        REQUIRE(vector.full() == false);

        vector.pop_back();

        REQUIRE(vector.size() == 32);
        REQUIRE(vector.capacity() == 64);
        REQUIRE(vector.empty() == false);
        REQUIRE(vector.full() == false);

        while (vector.size() > 0)
        {
            vector.pop_back();
        }

        REQUIRE(vector.size() == 0);
        REQUIRE(vector.capacity() == 64);
        REQUIRE(vector.empty() == true);
        REQUIRE(vector.full() == false);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Random Access", "[core::containers::pagedVector]")
    {
        PagedVector<uint32_t, 32> vector;

        for (auto i = 0; i < 32; ++i)
        {
            vector.push_back(i);
        }

        REQUIRE(vector[0] == 0);
        REQUIRE(vector[13] == 13);
        REQUIRE(vector[27] == 27);

        vector[27] = 127;

        REQUIRE(vector[27] == 127);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Fixed Access", "[core::containers::pagedVector]")
    {
        PagedVector<uint32_t, 32> vector;

        vector.push_back(1);

        REQUIRE(vector.front() == vector.back());

        vector.push_back(2);
        vector.push_back(3);

        REQUIRE(vector.front() == 1);
        REQUIRE(vector.back() == 3);

        vector.pop_back();

        REQUIRE(vector.front() == 1);
        REQUIRE(vector.back() == 2);

        vector.pop_back();

        REQUIRE(vector.front() == 1);
        REQUIRE(vector.front() == vector.back());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Iterator", "[core::containers::pagedVector]")
    {
        PagedVector<uint32_t, 32> vector;

        const auto count = 13;

        for (auto i = 0; i < count; ++i)
        {
            vector.push_back(i);
        }

        REQUIRE(vector.size() == count);
        REQUIRE(*(vector.begin()) == 0);

        size_t i = 0;

        for (auto x : vector)
        {
            REQUIRE(x == i++);
        }

        i = 0;

        for (auto iter = vector.begin(); iter != vector.end(); ++iter)
        {
            REQUIRE(*iter == i++);
        }
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Stable Memory", "[core::containers::pagedVector]")
    {
        // As comparison, use a standard vector and see how the memory address of element 0 changes.
        // std::vector<uint32_t> vector;
        // vector.reserve(4);

        PagedVector<uint32_t, 32> vector;

        vector.push_back(1337);
        uint32_t* ptr0 = &vector[0];

        for (auto i = 0; i < 1024; ++i)
        {
            vector.push_back(i);
        }

        uint32_t* ptr1 = &vector[0];

        REQUIRE(*ptr0 == 1337);
        REQUIRE(ptr0 == ptr1);
    } END_LITL_TEST_CASE

    namespace PushPopClassTest
    {
        class Foo
        {
        public:

            Foo(uint32_t ia, uint32_t ib, uint32_t* id)
                : a(ia), b(ib), destroyRecord(id)
            {

            }

            ~Foo()
            {
                if (destroyRecord != nullptr)
                {
                    (*destroyRecord)++;
                }
            }

            uint32_t a;
            uint32_t b;
            uint32_t* destroyRecord;
        };
    }

    LITL_TEST_CASE("Push-Pop Class", "[core::containers::pagedVector]")
    {
        PagedVector<PushPopClassTest::Foo> vector;
        uint32_t destroyCount = 0;

        {
            PushPopClassTest::Foo foo{ 5, 10, &destroyCount };
            vector.push_back(foo);

            REQUIRE(vector.size() == 1);
            REQUIRE(vector[0].a == 5);
            REQUIRE(vector[0].b == 10);

            vector.emplace_back(20, 30, &destroyCount);

            REQUIRE(vector.size() == 2);
            REQUIRE(vector[1].a == 20);
            REQUIRE(vector[1].b == 30);

            vector.pop_back();

            REQUIRE(vector.size() == 1);
            REQUIRE(destroyCount == 1);

            vector.pop_back();

            REQUIRE(vector.size() == 0);
            REQUIRE(destroyCount == 2);
        }

        // 2 in the container + 1 local to the scope that was pushed in.
        REQUIRE(destroyCount == 3);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Clear", "[core::containers::pagedVector]")
    {
        // Verify clear destroys all elements and resets state
        uint32_t destroyCount = 0;

        {
            PagedVector<PushPopClassTest::Foo> vector;

            for (auto i = 0; i < 10; ++i)
            {
                vector.emplace_back(i, i, &destroyCount);
            }

            REQUIRE(vector.size() == 10);
            REQUIRE(destroyCount == 0);

            vector.clear();

            REQUIRE(vector.size() == 0);
            REQUIRE(vector.capacity() == 0);
            REQUIRE(vector.empty() == true);
            REQUIRE(destroyCount == 10);

            // Verify the vector is reusable after clear
            vector.emplace_back(42, 42, &destroyCount);

            REQUIRE(vector.size() == 1);
            REQUIRE(vector[0].a == 42);
        }

        // Destructor cleans up the one remaining element
        REQUIRE(destroyCount == 11);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Clear Trivial", "[core::containers::pagedVector]")
    {
        // Verify clear works for trivially destructible types (hits the constexpr branch)
        PagedVector<uint32_t, 32> vector;

        for (auto i = 0; i < 100; ++i)
        {
            vector.push_back(i);
        }

        REQUIRE(vector.size() == 100);

        vector.clear();

        REQUIRE(vector.size() == 0);
        REQUIRE(vector.capacity() == 0);
        REQUIRE(vector.empty() == true);

        // Reusable after clear
        vector.push_back(7);

        REQUIRE(vector.size() == 1);
        REQUIRE(vector[0] == 7);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Reserve", "[core::containers::pagedVector]")
    {
        PagedVector<uint32_t, 32> vector;

        REQUIRE(vector.capacity() == 0);

        vector.reserve(1);

        REQUIRE(vector.capacity() == 32);
        REQUIRE(vector.size() == 0);
        REQUIRE(vector.empty() == true);

        vector.reserve(32);

        REQUIRE(vector.capacity() == 32);

        vector.reserve(33);

        REQUIRE(vector.capacity() == 64);
        REQUIRE(vector.size() == 0);

        // Reserve below current capacity is a no-op
        vector.reserve(10);

        REQUIRE(vector.capacity() == 64);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Move Semantics", "[core::containers::pagedVector]")
    {
        PagedVector<uint32_t, 32> vector;

        for (auto i = 0; i < 50; ++i)
        {
            vector.push_back(i);
        }

        uint32_t* stablePtr = &vector[0];

        // Move construct
        PagedVector<uint32_t, 32> moved(std::move(vector));

        REQUIRE(moved.size() == 50);
        REQUIRE(moved[0] == 0);
        REQUIRE(moved[49] == 49);
        REQUIRE(&moved[0] == stablePtr); // Pages transferred, not copied

        REQUIRE(vector.empty() == true);
        REQUIRE(vector.size() == 0);

        // Move assign
        PagedVector<uint32_t, 32> assigned;
        assigned.push_back(999);
        assigned = std::move(moved);

        REQUIRE(assigned.size() == 50);
        REQUIRE(assigned[0] == 0);
        REQUIRE(&assigned[0] == stablePtr);

        REQUIRE(moved.empty() == true);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Const Access", "[core::containers::pagedVector]")
    {
        PagedVector<uint32_t, 32> vector;

        for (auto i = 0; i < 10; ++i)
        {
            vector.push_back(i * 10);
        }

        const auto& constRef = vector;

        REQUIRE(constRef.size() == 10);
        REQUIRE(constRef[0] == 0);
        REQUIRE(constRef[9] == 90);
        REQUIRE(constRef.front() == 0);
        REQUIRE(constRef.back() == 90);
        REQUIRE(constRef.empty() == false);

        // Const iteration
        size_t i = 0;

        for (auto const& x : constRef)
        {
            REQUIRE(x == i * 10);
            ++i;
        }

        REQUIRE(i == 10);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Iterator Arithmetic", "[core::containers::pagedVector]")
    {
        PagedVector<uint32_t, 32> vector;

        for (auto i = 0; i < 64; ++i)
        {
            vector.push_back(i);
        }

        auto it = vector.begin();

        // Operator+, operator[]
        REQUIRE(*(it + 5) == 5);
        REQUIRE(it[10] == 10);

        // Operator+=
        it += 20;

        REQUIRE(*it == 20);

        // Operator-=
        it -= 5;

        REQUIRE(*it == 15);

        // Difference
        auto diff = vector.end() - vector.begin();

        REQUIRE(diff == 64);

        // Comparisons
        REQUIRE(vector.begin() < vector.end());
        REQUIRE(vector.end() > vector.begin());
        REQUIRE(vector.begin() <= vector.begin());
        REQUIRE(vector.begin() >= vector.begin());

        // n + iterator (commutative)
        REQUIRE(*(3 + vector.begin()) == 3);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Cross Page Access", "[core::containers::pagedVector]")
    {
        // Verify correctness at page boundaries
        PagedVector<uint32_t, 4> vector;

        for (auto i = 0; i < 13; ++i)
        {
            vector.push_back(i * 100);
        }

        REQUIRE(vector.capacity() == 16); // 4 pages of 4

        // Verify values at page boundaries
        REQUIRE(vector[3] == 300);  // Last element of page 0
        REQUIRE(vector[4] == 400);  // First element of page 1
        REQUIRE(vector[7] == 700);  // Last element of page 1
        REQUIRE(vector[8] == 800);  // First element of page 2
        REQUIRE(vector[12] == 1200); // Last element, page 3

        // Iterate across all page boundaries
        size_t i = 0;

        for (auto x : vector)
        {
            REQUIRE(x == i * 100);
            ++i;
        }
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Pop Back On Empty", "[core::containers::pagedVector]")
    {
        PagedVector<uint32_t, 32> vector;

        // Should be a safe no-op
        vector.pop_back();

        REQUIRE(vector.size() == 0);
        REQUIRE(vector.empty() == true);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Destructor Calls On Scope Exit", "[core::containers::pagedVector]")
    {
        // Verify the destructor path (not clear()) also destroys all elements
        uint32_t destroyCount = 0;

        {
            PagedVector<PushPopClassTest::Foo> vector;

            for (auto i = 0; i < 5; ++i)
            {
                vector.emplace_back(i, i, &destroyCount);
            }
        }

        REQUIRE(destroyCount == 5);
    } END_LITL_TEST_CASE
}