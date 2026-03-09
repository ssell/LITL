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

    auto job = jobPool.createJob(0, jobTest, &jobCount);

    REQUIRE(job->version == jobPool.version());

    job->func(job, 0);

    REQUIRE(jobCount == 1);
}

TEST_CASE("Pool Reset", "[core::work::jobPool]")
{
    LITL::Core::JobPool jobPool{ 1 };
    uint32_t jobCount = 0;

    auto job0 = jobPool.createJob(0, jobTest, &jobCount);
    auto job1 = jobPool.createJob(0, jobTest, &jobCount);

    REQUIRE(job0->version == jobPool.version());
    REQUIRE(job1->version == jobPool.version());

    job0->func(job0, 0);
    job1->func(job1, 0);

    REQUIRE(jobCount == 2);

    jobPool.reset();        // reset offsets and increment version

    REQUIRE(job0->version != jobPool.version());
    REQUIRE(job1->version != jobPool.version());

    auto job2 = jobPool.createJob(0, jobTest, &jobCount);

    REQUIRE(job2->version == jobPool.version());
    REQUIRE(job2->version != job1->version);

    job2->func(job2, 0);

    REQUIRE(jobCount == 3);
    REQUIRE(job2 == job0);      // Memory address for job2 reused the now outdated memory address of job0
}