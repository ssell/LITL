#include "tests.hpp"
#include "litl-core/job/job.hpp"

namespace LITL::Core::Tests
{
    LITL_TEST_CASE("JobHandle Copy", "[core::job::job]")
    {
        Job job;
        JobHandle handle0;
        JobHandle handle1;

        REQUIRE(handle0.job == nullptr);

        handle0.job = &job;

        REQUIRE(handle0.job != nullptr);

        handle1 = handle0;

        REQUIRE(handle1.job == handle0.job);
        REQUIRE(handle1.job == &job);
    } END_LITL_TEST_CASE
}