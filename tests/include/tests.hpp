#ifndef LITL_TESTS_H__
#define LITL_TESTS_H__

#include <chrono>
#include <catch2/catch_test_macros.hpp>
#include <iomanip>
#include <iostream>

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


#endif