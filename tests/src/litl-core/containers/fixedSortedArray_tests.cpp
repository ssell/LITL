#include "tests.hpp"
#include "litl-core/containers/fixedSortedArray.hpp"

namespace litl::tests
{
    namespace
    {
        struct SortStruct
        {
            /// <summary>
            /// Required for FixedSortedArray. Custom equality for the dedupe (unique).
            /// </summary>
            /// <param name="other"></param>
            /// <returns></returns>
            bool operator==(SortStruct const& other) const
            {
                return ((a == other.a) && (b == other.b));
            }

            /// <summary>
            /// Required for FixedSortedArray. Custom spaceship for std::sort used by FixedSortedArray
            /// </summary>
            /// <param name="a"></param>
            /// <param name="b"></param>
            /// <returns></returns>
            auto operator<=>(SortStruct const& other) const
            {
                return ((a + b) <=> (other.a + other.b));
            }

            uint32_t a{ 0 };
            uint32_t b{ 0 };
        };
    }

    LITL_TEST_CASE("Basic", "[core::containers::fixedSortedArray]")
    {
        std::vector<uint32_t> source({ 33, 16, 88, 1, 3, 987, 1337, 6, 88, 16 });
        FixedSortedArray<uint32_t, 32> array(source);

        REQUIRE(array.capacity() == 32);
        REQUIRE(array.size() == 8);                 // 10 original - 2 dupes (88 and 16)

        REQUIRE(*(array.begin() + 0) == 1);
        REQUIRE(*(array.begin() + 1) == 3);
        REQUIRE(*(array.begin() + 2) == 6);
        REQUIRE(*(array.begin() + 3) == 16);
        REQUIRE(*(array.begin() + 4) == 33);
        REQUIRE(*(array.begin() + 5) == 88);
        REQUIRE(*(array.begin() + 6) == 987);
        REQUIRE(*(array.begin() + 7) == 1337);
    } LITL_END_TEST_CASE
        
    LITL_TEST_CASE("Custom", "[core::containers::fixedSortedArray]")
    {
        std::vector<SortStruct> source({ { 0, 1000 }, {5, 5}, {50, 0}, {88, 88}, {5, 5} });
        FixedSortedArray<SortStruct> array(source);

        REQUIRE(array.capacity() == 64);
        REQUIRE(array.size() == 4);                 // 5 original - 1 dupe ({5, 5})

        auto& at0 = *(array.begin() + 0);
        auto& at1 = *(array.begin() + 1);
        auto& at2 = *(array.begin() + 2);
        auto& at3 = *(array.begin() + 3);

        // expected order of: [{5, 5}, {50, 0}, {88, 88}, {0, 1000}]

        REQUIRE(at0.a == 5);
        REQUIRE(at0.b == 5);

        REQUIRE(at1.a == 50);
        REQUIRE(at1.b == 0);

        REQUIRE(at2.a == 88);
        REQUIRE(at2.b == 88);

        REQUIRE(at3.a == 0);
        REQUIRE(at3.b == 1000);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Initializer List", "[core::containers::fixedSortedArray]")
    {
        FixedSortedArray<uint32_t> array({ 8, 6, 1, 1, 3, 9, 15, 4 });

        REQUIRE(array.size() == 7);

        REQUIRE(*(array.begin() + 0) == 1);
        REQUIRE(*(array.begin() + 1) == 3);
        REQUIRE(*(array.begin() + 2) == 4);
        REQUIRE(*(array.begin() + 3) == 6);
        REQUIRE(*(array.begin() + 4) == 8);
        REQUIRE(*(array.begin() + 5) == 9);
        REQUIRE(*(array.begin() + 6) == 15);
    } LITL_END_TEST_CASE
}