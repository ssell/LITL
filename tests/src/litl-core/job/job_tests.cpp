#include "tests.hpp"
#include "litl-core/job/job.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("JobHandle Copy", "[core::job::job]")
    {
        Job job;
        JobHandle handle0;
        JobHandle handle1(6, 1337);

        REQUIRE(handle0.isNull() == true);
        REQUIRE(handle1.isNull() == false);

        REQUIRE(handle0.pool() == 0);
        REQUIRE(handle0.job() == 0);

        REQUIRE(handle1.pool() == 6);
        REQUIRE(handle1.job() == 1337);

        handle0 = handle1;

        REQUIRE(handle0.pool() == 6);
        REQUIRE(handle0.job() == 1337);
    } END_LITL_TEST_CASE
}