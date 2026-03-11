#include <catch2/catch_test_macros.hpp>
#include "litl-core/work/job.hpp"

TEST_CASE("JobHandle Copy", "[core::work::job]")
{
    LITL::Core::Job job;
    LITL::Core::JobHandle handle0;
    LITL::Core::JobHandle handle1;

    REQUIRE(handle0.job == nullptr);

    handle0.job = &job;

    REQUIRE(handle0.job != nullptr);

    handle1 = handle0;

    REQUIRE(handle1.job == handle0.job);
    REQUIRE(handle1.job == &job);    
}