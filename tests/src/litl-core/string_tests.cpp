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

    LITL_TEST_CASE("isWhitespace", "[core::string]")
    {
        REQUIRE(isWhitespace('a') == false);
        REQUIRE(isWhitespace(' ') == true);
        REQUIRE(isWhitespace('3') == false);
        REQUIRE(isWhitespace('\t') == true);
        REQUIRE(isWhitespace('\v') == true);
        REQUIRE(isWhitespace('\f') == true);
        REQUIRE(isWhitespace('\r') == true);
        REQUIRE(isWhitespace('\n') == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("findFirstWhitespace", "[core::string]")
    {
        REQUIRE(findFirstWhitespace(" apples") == 0ull);
        REQUIRE(findFirstWhitespace("  apples") == 0ull);
        REQUIRE(findFirstWhitespace("apples are not oranges") == 6ull);
        REQUIRE(findFirstWhitespace("apples") == std::string_view::npos);
        REQUIRE(findFirstWhitespace("      ") == 0ull);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("findFirstNonWhitespace", "[core::string]")
    {
        REQUIRE(findFirstNonWhitespace(" apples") == 1ull);
        REQUIRE(findFirstNonWhitespace("  apples") == 2ull);
        REQUIRE(findFirstNonWhitespace("apples are not oranges") == 0ull);
        REQUIRE(findFirstNonWhitespace("apples") == 0ull);
        REQUIRE(findFirstNonWhitespace("      ") == std::string_view::npos);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("stringsEqual", "[core::string]")
    {
        REQUIRE(stringsEquals("apple", "apple", true) == true);
        REQUIRE(stringsEquals("apple", "apple", false) == true);
        REQUIRE(stringsEquals("apple", "APPLE", true) == true);
        REQUIRE(stringsEquals("apple", "APPLE", false) == false);
        REQUIRE(stringsEquals("APPLE", "apple", true) == true);
        REQUIRE(stringsEquals("APPLE", "apple", false) == false);
        REQUIRE(stringsEquals("APPLE", "APPLE", true) == true);
        REQUIRE(stringsEquals("APPLE", "APPLE", false) == true);
        REQUIRE(stringsEquals(" apple", "apple", true) == false);
        REQUIRE(stringsEquals("apple ", "apple", true) == false);
        REQUIRE(stringsEquals(" apple", "apple", false) == false);
        REQUIRE(stringsEquals("apple ", "apple", false) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("stringsEqualFirstLowercase", "[core::string]")
    {
        REQUIRE(stringsEqualFirstLowercase("apple", "apple") == true);
        REQUIRE(stringsEqualFirstLowercase("apple", "APPlE") == true);
        REQUIRE(stringsEqualFirstLowercase("apple", "apple ") == false);
        REQUIRE(stringsEqualFirstLowercase("APPLE", "APPLE") == false);         // Function requires the first string to be lowercase already as an optimization
    } LITL_END_TEST_CASE
}