#include <catch2/catch_test_macros.hpp>
#include "litl-core/containers/pagedVector.hpp"

TEST_CASE("Push-Pop", "[core::containers::pagedVector]")
{
    LITL::Core::PagedVector<uint32_t> vector(32);

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
}

TEST_CASE("Random Access", "[core::containers::pagedVector]")
{
    LITL::Core::PagedVector<uint32_t> vector(32);

    for (auto i = 0; i < 32; ++i)
    {
        vector.push_back(i);
    }

    REQUIRE(vector[0] == 0);
    REQUIRE(vector[13] == 13);
    REQUIRE(vector.at(27) == 27);

    vector[27] = 127;

    REQUIRE(vector[27] == 127);
}

TEST_CASE("Fixed Access", "[core::containers::pagedVector]")
{
    LITL::Core::PagedVector<uint32_t> vector(32);

    REQUIRE_THROWS(vector.front());
    REQUIRE_THROWS(vector.back());

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
}

TEST_CASE("Iterator", "[core::containers::pagedVector]")
{
    LITL::Core::PagedVector<uint32_t> vector(32);

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
}

TEST_CASE("Stable Memory", "[core::containers::pagedVector]")
{
    // As comparison, use a standard vector and see how the memory address of element 0 changes.
    // std::vector<uint32_t> vector;
    // vector.reserve(4);

    LITL::Core::PagedVector<uint32_t> vector(4);
    
    vector.push_back(1337);
    uint32_t* ptr0 = &vector.at(0);

    for (auto i = 0; i < 1024; ++i)
    {
        vector.push_back(i);
    }

    uint32_t* ptr1 = &vector.at(0);

    REQUIRE(*ptr0 == 1337);
    REQUIRE(ptr0 == ptr1);
}