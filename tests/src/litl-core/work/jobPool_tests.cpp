#include <catch2/catch_test_macros.hpp>
#include "litl-core/work/jobPool.hpp"

void jobTest(LITL::Core::Job* job, uint32_t threadIndex)
{
    auto* jobCount = static_cast<uint32_t*>(job->data);
    (*jobCount)++;
}

TEST_CASE("Single Job", "[core::work::jobPool]")
{
    LITL::Core::JobPool jobPool{ 1 };
    uint32_t jobCount = 0;

    auto handle = jobPool.createJob(0, jobTest, &jobCount);

    REQUIRE(handle.version == jobPool.version());

    handle.job->func(handle.job, 0);

    REQUIRE(jobCount == 1);
}

TEST_CASE("Pool Reset", "[core::work::jobPool]")
{
    LITL::Core::JobPool jobPool{ 1 };
    uint32_t jobCount = 0;

    auto handle0 = jobPool.createJob(0, jobTest, &jobCount);
    auto handle1 = jobPool.createJob(0, jobTest, &jobCount);

    REQUIRE(handle0.version == jobPool.version());
    REQUIRE(handle0.valid(jobPool.version()) == true);

    REQUIRE(handle1.version == jobPool.version());
    REQUIRE(handle1.valid(jobPool.version()) == true);

    handle0.job->func(handle0.job, 0);
    handle1.job->func(handle1.job, 0);

    REQUIRE(jobCount == 2);

    jobPool.sync();        // Clear current jobs, reset job pools, and increment version.

    REQUIRE(handle0.version != jobPool.version());
    REQUIRE(handle0.valid(jobPool.version()) == false);

    REQUIRE(handle1.version != jobPool.version());
    REQUIRE(handle1.valid(jobPool.version()) == false);

    // Create a third job. This job should use the same memory address as the first (now reset) job.
    // Though their addresses are the same, their versions should differ.
    auto handle2 = jobPool.createJob(0, jobTest, &jobCount);

    REQUIRE(handle2.version == jobPool.version());
    REQUIRE(handle2.valid(jobPool.version()) == true);
    REQUIRE(handle2.version != handle1.version);
    REQUIRE(handle2.job == handle0.job);

    handle2.job->func(handle2.job, 0);

    REQUIRE(jobCount == 3);
}