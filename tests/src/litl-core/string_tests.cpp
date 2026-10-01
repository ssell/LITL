#include "tests.hpp"
#include "litl-core/string.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("toLowercase", "[core::string]")
    {
        REQUIRE(toLowercase("APPLES") == "apples");
        REQUIRE(toLowercase("aPpLeS") == "apples");
        REQUIRE(toLowercase(" uatKaM     F71SSmD9PP !@3 BQU4ignvQBHu0Ojq ? a35ACx  ") == " uatkam     f71ssmd9pp !@3 bqu4ignvqbhu0ojq ? a35acx  ");

    } LITL_END_TEST_CASE

    LITL_TEST_CASE("trimLeadingWhitespace", "[core::string]")
    {
        REQUIRE(trimLeadingWhitespace("sentence") == "sentence");
        REQUIRE(trimLeadingWhitespace("   sentence") == "sentence");
        REQUIRE(trimLeadingWhitespace("  sentence  ") == "sentence  ");
    } LITL_END_TEST_CASE
}