#include "tests.hpp"
#include "litl-core/file.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("Is Reserved Name (Case Sensitive)", "[core::file]")
    {
        REQUIRE(File::IsReservedFileName("con") == true);
        REQUIRE(File::IsReservedFileName("CON") == false);
        REQUIRE(File::IsReservedFileName("nul") == true);
        REQUIRE(File::IsReservedFileName("NUL") == false);
        REQUIRE(File::IsReservedFileName("null") == false);
        REQUIRE(File::IsReservedFileName("aux") == true);
        REQUIRE(File::IsReservedFileName("AUX") == false);
        REQUIRE(File::IsReservedFileName("com3") == true);
        REQUIRE(File::IsReservedFileName("COm3") == false);
        REQUIRE(File::IsReservedFileName("lpt6") == true);
        REQUIRE(File::IsReservedFileName("LpT6") == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Is Reserved Name (Case Insensitive)", "[core::file]")
    {
        REQUIRE(File::IsReservedFileNameCaseInsensitive("con") == true);
        REQUIRE(File::IsReservedFileNameCaseInsensitive("CON") == true);
        REQUIRE(File::IsReservedFileNameCaseInsensitive("nul") == true);
        REQUIRE(File::IsReservedFileNameCaseInsensitive("NUL") == true);
        REQUIRE(File::IsReservedFileNameCaseInsensitive("null") == false);
        REQUIRE(File::IsReservedFileNameCaseInsensitive("aux") == true);
        REQUIRE(File::IsReservedFileNameCaseInsensitive("AUX") == true);
        REQUIRE(File::IsReservedFileNameCaseInsensitive("com3") == true);
        REQUIRE(File::IsReservedFileNameCaseInsensitive("COm3") == true);
        REQUIRE(File::IsReservedFileNameCaseInsensitive("lpt6") == true);
        REQUIRE(File::IsReservedFileNameCaseInsensitive("LpT6") == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Sanitize Filename", "[core::file]")
    {
        REQUIRE(File::SanitizeFilename("foo") == "foo");
        REQUIRE(File::SanitizeFilename(" foo/bar ") == "foo_bar");
        REQUIRE(File::SanitizeFilename("is foo   equal to bar???   ") == "is-foo-equal-to-bar___");
        REQUIRE(File::SanitizeFilename("") == "");
        REQUIRE(File::SanitizeFilename("/\\:*?\"<>|") == "_________");
        REQUIRE(File::SanitizeFilename("                           whitespaces            are      cool!") == "whitespaces-are-cool!");
    } LITL_END_TEST_CASE
}