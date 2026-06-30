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
}