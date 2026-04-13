#ifndef LITL_TESTS_H__
#define LITL_TESTS_H__

#include <chrono>
#include <catch2/catch_test_macros.hpp>
#include <iomanip>
#include <iostream>

#include "litl-core/assert.hpp"

#define LITL_TEST_CASE_CAPTURE(A, B) auto test_name__ = A;                                                                          \
        auto test_group__ = B;                                                                                                      \
        std::cout << test_group__ << " " << test_name__ << " ...";                                                                  \
        auto test_start_time__ = std::chrono::steady_clock::now();

#define END_LITL_TEST_CASE auto test_end_time__ = std::chrono::steady_clock::now();                                                 \
        auto test_elapsed_ms__ = std::chrono::duration<double, std::milli>(test_end_time__ - test_start_time__);                    \
        std::cout << std::fixed << std::setprecision(3) << " done! (" << test_elapsed_ms__.count() << "ms)\n";                      \
        }

#define LITL_TEST_CASE(A, B) TEST_CASE(A, B) { LITL_TEST_CASE_CAPTURE(A, B)
#define LITL_TEST_CASE_METHOD(M, A, B) TEST_CASE_METHOD(M, A, B) { LITL_TEST_CASE_CAPTURE(A, B)

namespace litl::tests
{
    using EmptyAssertHandler = std::function<void()>;

    /// <summary>
    /// Utility for catching asserts (both warning and fatal) to test that they are being thrown.
    /// </summary>
    struct TestAssertCallback
    {
        TestAssertCallback(AssertHandler userAssertHandler)
            : wasAbortEnabled(internal::Assert::getAbortEnabled()), defaultWarningHandler(internal::Assert::getWarningAssertHandler()), defaultFatalHandler(internal::Assert::getFatalAssertHandler()), userHandler(userAssertHandler)
        {
            internal::Assert::getWarningAssertHandler() = [this](char const* expression, char const* message, char const* file, uint32_t line)
                {
                    userHandler(expression, message, file, line);
                };

            internal::Assert::getFatalAssertHandler() = [this](char const* expression, char const* message, char const* file, uint32_t line)
                {
                    userHandler(expression, message, file, line);
                };

            internal::Assert::setAbortEnabled(false);
        }

        TestAssertCallback(EmptyAssertHandler userAssertHandler)
            : wasAbortEnabled(internal::Assert::getAbortEnabled()), defaultWarningHandler(internal::Assert::getWarningAssertHandler()), defaultFatalHandler(internal::Assert::getFatalAssertHandler()), userEmptyHandler(userAssertHandler)
        {
            internal::Assert::getWarningAssertHandler() = [this](char const* expression, char const* message, char const* file, uint32_t line)
                {
                    userEmptyHandler();
                };

            internal::Assert::getFatalAssertHandler() = [this](char const* expression, char const* message, char const* file, uint32_t line)
                {
                    userEmptyHandler();
                };

            internal::Assert::setAbortEnabled(false);
        }

        ~TestAssertCallback()
        {
            internal::Assert::setAbortEnabled(wasAbortEnabled);
            internal::Assert::getWarningAssertHandler() = defaultWarningHandler;
            internal::Assert::getFatalAssertHandler() = defaultFatalHandler;
        }

    private:

        bool wasAbortEnabled;

        AssertHandler defaultWarningHandler;
        AssertHandler defaultFatalHandler;

        AssertHandler userHandler;
        EmptyAssertHandler userEmptyHandler;
    };
}

#define LITL_START_ASSERT_CAPTURE                                                                   \
    {                                                                                               \
        bool assert_caught__ = false;                                                               \
        TestAssertCallback assert_caught_cb__([&assert_caught__]() { assert_caught__ = true; });

#define LITL_END_ASSERT_CAPTURE                                                                     \
        REQUIRE(assert_caught__ == true);                                                           \
    }

#endif