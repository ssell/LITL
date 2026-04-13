#include "tests.hpp"
#include "litl-core/assert.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("assert warning LITL_ASSERT", "[core::assert]")
    {
        bool caughtAssertion = false;
        std::string caughtExpression = "";
        std::string caughtMessage = "";
        std::string caughtFile = "";
        uint32_t caughtLine = 0;

        TestAssertCallback assertCallback([&](char const* expression, char const* message, char const* file, uint32_t line)
            {
                caughtAssertion = true;
                caughtExpression = expression;
                caughtMessage = (message == nullptr ? "" : message);
                caughtFile = file;
                caughtLine = line;
            });

        LITL_ASSERT((true == false), );

        REQUIRE(caughtAssertion == true);
        REQUIRE(caughtExpression == "(true == false)");
        REQUIRE(caughtMessage == "");
        REQUIRE(caughtFile.find("assert_tests.cpp") > 0);
        REQUIRE(caughtLine > 0);

    } END_LITL_TEST_CASE

    LITL_TEST_CASE("assert warning LITL_ASSERT_MSG", "[core::assert]")
    {
        bool caughtAssertion = false;
        std::string caughtExpression = "";
        std::string caughtMessage = "";
        std::string caughtFile = "";
        uint32_t caughtLine = 0;

        TestAssertCallback assertCallback([&](char const* expression, char const* message, char const* file, uint32_t line)
            {
                caughtAssertion = true;
                caughtExpression = expression;
                caughtMessage = (message == nullptr ? "" : message);
                caughtFile = file;
                caughtLine = line;
            });

        LITL_ASSERT_MSG((true == false), "true does not equal false!", );

        REQUIRE(caughtAssertion == true);
        REQUIRE(caughtExpression == "(true == false)");
        REQUIRE(caughtMessage == "true does not equal false!");
        REQUIRE(caughtFile.find("assert_tests.cpp") > 0);
        REQUIRE(caughtLine > 0);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("assert LITL_FATAL_ASSERT", "[core::assert]")
    {
        bool caughtAssertion = false;
        std::string caughtExpression = "";
        std::string caughtMessage = "";
        std::string caughtFile = "";
        uint32_t caughtLine = 0;

        TestAssertCallback assertCallback([&](char const* expression, char const* message, char const* file, uint32_t line)
            {
                caughtAssertion = true;
                caughtExpression = expression;
                caughtMessage = (message == nullptr ? "" : message);
                caughtFile = file;
                caughtLine = line;
            });

        LITL_FATAL_ASSERT((true == false));

        REQUIRE(caughtAssertion == true);
        REQUIRE(caughtExpression == "(true == false)");
        REQUIRE(caughtMessage == "");
        REQUIRE(caughtFile.find("assert_tests.cpp") > 0);
        REQUIRE(caughtLine > 0);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("assert LITL_FATAL_ASSERT_MSG", "[core::assert]")
    {
        bool caughtAssertion = false;
        std::string caughtExpression = "";
        std::string caughtMessage = "";
        std::string caughtFile = "";
        uint32_t caughtLine = 0;

        TestAssertCallback assertCallback([&](char const* expression, char const* message, char const* file, uint32_t line)
            {
                caughtAssertion = true;
                caughtExpression = expression;
                caughtMessage = (message == nullptr ? "" : message);
                caughtFile = file;
                caughtLine = line;
            });

        LITL_FATAL_ASSERT_MSG((true == false), "true does not equal false!");

        REQUIRE(caughtAssertion == true);
        REQUIRE(caughtExpression == "(true == false)");
        REQUIRE(caughtMessage == "true does not equal false!");
        REQUIRE(caughtFile.find("assert_tests.cpp") > 0);
        REQUIRE(caughtLine > 0);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("helper macro", "[core::assert]")
    {
        LITL_START_ASSERT_CAPTURE
            LITL_ASSERT((true == false), );
        LITL_END_ASSERT_CAPTURE
    } END_LITL_TEST_CASE
}