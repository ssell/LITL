#include "tests.hpp"
#include "litl-core/containers/fixedSortedArray.hpp"

namespace LITL::Core::Tests
{
    LITL_TEST_CASE("Basic", "[core::containers::fixedSortedArray]")
    {
        // 10 items, but 2 dupes (88 and 16)
        std::vector<uint32_t> source({ 33, 16, 88, 1, 3, 987, 1337, 6, 88, 16 });
        FixedSortedArray<32> array(source);

        REQUIRE(array.capacity() == 32);
        REQUIRE(array.size() == 8);

        REQUIRE(*(array.begin() + 0) == 1);
        REQUIRE(*(array.begin() + 1) == 3);
        REQUIRE(*(array.begin() + 2) == 6);
        REQUIRE(*(array.begin() + 3) == 16);
        REQUIRE(*(array.begin() + 4) == 33);
        REQUIRE(*(array.begin() + 5) == 88);
        REQUIRE(*(array.begin() + 6) == 987);
        REQUIRE(*(array.begin() + 7) == 1337);


    } END_LITL_TEST_CASE
}