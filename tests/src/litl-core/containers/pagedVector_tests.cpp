#include <catch2/catch_test_macros.hpp>
#include "litl-core/containers/pagedVector.hpp"

TEST_CASE("Push-Pop", "[core::containers::pagedVector]")
{
    LITL::Core::PagedVector<uint32_t, 32> vector;

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