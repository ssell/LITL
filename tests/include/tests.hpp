#ifndef LITL_TESTS_H__
#define LITL_TESTS_H__

#include <catch2/catch_test_macros.hpp>

#define LITL_TEST_CASE_CAPTURE(A, B) \
        auto test_name__ = A; \
        auto test_group__ = B; \
        printf("%s %s ...", test_group__, test_name__);

#define LITL_TEST_CASE(A, B) TEST_CASE(A, B) { LITL_TEST_CASE_CAPTURE(A, B)
#define LITL_TEST_CASE_METHOD(M, A, B) TEST_CASE_METHOD(M, A, B) { LITL_TEST_CASE_CAPTURE(A, B)
#define END_LITL_TEST_CASE printf(" done!\n"); }

#endif