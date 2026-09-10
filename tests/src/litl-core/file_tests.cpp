#include "tests.hpp"
#include "litl-core/file.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("IsReservedFileNameCaseInsensitive", "[core::file]")
    {
        REQUIRE(File::IsReservedFileName("con") == true);
        REQUIRE(File::IsReservedFileName("CON") == true);
        REQUIRE(File::IsReservedFileName("nul") == true);
        REQUIRE(File::IsReservedFileName("NUL") == true);
        REQUIRE(File::IsReservedFileName("null") == false);
        REQUIRE(File::IsReservedFileName("aux") == true);
        REQUIRE(File::IsReservedFileName("AUX") == true);
        REQUIRE(File::IsReservedFileName("com3") == true);
        REQUIRE(File::IsReservedFileName("COm3") == true);
        REQUIRE(File::IsReservedFileName("lpt6") == true);
        REQUIRE(File::IsReservedFileName("LpT6") == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Sanitize Filename", "[core::file]")
    {
        REQUIRE(File::SanitizeFilename("foo") == "foo");
        REQUIRE(File::SanitizeFilename(" foo/bar ") == "foo_bar");
        REQUIRE(File::SanitizeFilename("is foo   equal to bar???   ") == "is-foo-equal-to-bar___");
        REQUIRE(File::SanitizeFilename("") == "");
        REQUIRE(File::SanitizeFilename("/\\:*?\"<>|") == "_________");
        REQUIRE(File::SanitizeFilename("                           whitespaces            are      cool!") == "whitespaces-are-cool!");
        REQUIRE(File::SanitizeFilename("a ?c") == "a-_c");

    } LITL_END_TEST_CASE
}