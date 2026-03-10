#include <catch2/catch_test_macros.hpp>
#include "litl-core/work/jobPool.hpp"

namespace
{
    void jobTest(LITL::Core::Job* job, uint32_t threadIndex)
    {
        auto* jobsRun = static_cast<uint32_t*>(job->data);
        (*jobsRun)++;
    }
}

TEST_CASE("Single Job", "[core::work::jobPool]")
{
    LITL::Core::JobPool jobPool{ 1 };
    uint32_t jobsRun = 0;

    auto handle = jobPool.createJob(0, jobTest, &jobsRun);

    REQUIRE(handle.version == jobPool.version());

    handle.job->func(handle.job, 0);

    REQUIRE(jobsRun == 1);
}

TEST_CASE("Pool Reset", "[core::work::jobPool]")
{
    LITL::Core::JobPool jobPool{ 1 };
    uint32_t jobsRun = 0;

    auto handle0 = jobPool.createJob(0, jobTest, &jobsRun);
    auto handle1 = jobPool.createJob(0, jobTest, &jobsRun);

    REQUIRE(handle0.version == jobPool.version());
    REQUIRE(handle0.valid(jobPool.version()) == true);

    REQUIRE(handle1.version == jobPool.version());
    REQUIRE(handle1.valid(jobPool.version()) == true);

    handle0.job->func(handle0.job, 0);
    handle1.job->func(handle1.job, 0);

    REQUIRE(jobsRun == 2);

    jobPool.sync();        // Clear current jobs, reset job pools, and increment version.

    REQUIRE(handle0.version != jobPool.version());
    REQUIRE(handle0.valid(jobPool.version()) == false);

    REQUIRE(handle1.version != jobPool.version());
    REQUIRE(handle1.valid(jobPool.version()) == false);

    // Create a third job. This job should use the same memory address as the first (now reset) job.
    // Though their addresses are the same, their versions should differ.
    auto handle2 = jobPool.createJob(0, jobTest, &jobsRun);

    REQUIRE(handle2.version == jobPool.version());
    REQUIRE(handle2.valid(jobPool.version()) == true);
    REQUIRE(handle2.version != handle1.version);
    REQUIRE(handle2.job == handle0.job);

    handle2.job->func(handle2.job, 0);

    REQUIRE(jobsRun == 3);
}

TEST_CASE("Many Jobs", "[core::work::jobPool]")
{
    // Allocate enough jobs to (a) exceed the number in the thread-local pools (1024 atm) and fill multiple pages of the global pool (1024 per global page atm)
    constexpr uint32_t jobsCount = 8192;

    LITL::Core::JobPool jobPool{ 1 };
    std::vector<LITL::Core::JobHandle> handles;
    handles.reserve(jobsCount);

    uint32_t jobsRun = 0;

    auto startAllocate0 = std::chrono::steady_clock::now();

    for (auto i = 0; i < jobsCount; ++i)
    {
        handles.push_back(jobPool.createJob(0, jobTest, &jobsRun));
    }

    auto startRun0 = std::chrono::steady_clock::now();

    for (auto i = 0; i < jobsCount; ++i)
    {
        handles[i].job->func(handles[i].job, 0);
    }

    auto endRun0 = std::chrono::steady_clock::now();
    auto timeToAllocate0 = (startRun0 - startAllocate0);
    auto timeToRun0 = (endRun0 - startRun0);

    REQUIRE(jobsRun == jobsCount);

    jobPool.sync();

    auto startAllocate1 = std::chrono::steady_clock::now();

    for (auto i = 0; i < jobsCount; ++i)
    {
        jobPool.createJob(0, jobTest, &jobsRun);
    }

    auto startRun1 = std::chrono::steady_clock::now();

    for (auto i = 0; i < jobsCount; ++i)
    {
        handles[i].job->func(handles[i].job, 0);
    }

    auto endRun1 = std::chrono::steady_clock::now();
    auto timeToAllocate1 = (startRun1 - startAllocate1);
    auto timeToRun1 = (endRun1 - startRun1);

    REQUIRE(jobsRun == jobsCount * 2);
}