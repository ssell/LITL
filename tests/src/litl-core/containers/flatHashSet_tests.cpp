#include <cstdint>

#include "tests.hpp"
#include "litl-core/containers/flatHashSet.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("Basic Operation", "[core::containers::flatHashSet]")
    {
        FlatHashSet<uint32_t> set{};

        REQUIRE(set.size() == 0ull);

        set.insert(55u);

        REQUIRE(set.size() == 1ull);
        REQUIRE(set.contains(55u) == true);

        set.insert(55u);

        REQUIRE(set.size() == 1ull);

        set.insert(56u);

        REQUIRE(set.size() == 2ull);
        REQUIRE(set.contains(56u) == true);

        set.erase(55u);

        REQUIRE(set.size() == 1ull);
        REQUIRE(set.contains(55u) == false);
        REQUIRE(set.contains(56u) == true);

        set.insert(100u);

        REQUIRE(set.size() == 2ull);
        REQUIRE(set.contains(100u) == true);
        REQUIRE(set.contains(55u) == false);
        REQUIRE(set.contains(56u) == true);

        set.clear();

        REQUIRE(set.size() == 0ull);
        REQUIRE(set.contains(100u) == false);
        REQUIRE(set.contains(55u) == false);
        REQUIRE(set.contains(56u) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Deduplication", "[core::containers::flatHashSet]")
    {
        FlatHashSet<uint32_t> set{};

        // Add [1, 2, 2, 3, 3, 3, 4, 4, 4, 4, ..., 9]
        for (uint32_t i = 0u; i < 10u; ++i)
        {
            for (uint32_t j = 0u; j < i; ++j)
            {
                set.insert(i);
            }
        }

        REQUIRE(set.size() == 9ull);        // 1 to 9 with no dupes

        for (uint32_t i = 1u; i < 10u; ++i)
        {
            REQUIRE(set.contains(i));
        }

        REQUIRE(set.erase(0u) == false);
        REQUIRE(set.erase(1u) == true);

        REQUIRE(set.size() == 8ull);

        REQUIRE(set.empty() == false);
        set.clear();
        REQUIRE(set.empty() == true);
    } LITL_END_TEST_CASE
}