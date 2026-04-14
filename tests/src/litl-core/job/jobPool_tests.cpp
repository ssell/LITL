#include "tests.hpp"
#include "litl-core/job/jobPool.hpp"

namespace litl::tests
{
    namespace
    {
        void jobTest(Job* job)
        {
            auto* jobsRun = static_cast<uint32_t*>(job->data);
            (*jobsRun)++;
        }
    }

    LITL_TEST_CASE("Single Job", "[core::job::jobPool]")
    {
        JobPool jobPool{ 1 };
        uint32_t jobsRun = 0;

        auto handle = jobPool.createJob(0, jobTest, &jobsRun);
        auto job = jobPool.resolve(handle);

        REQUIRE(handle.isNull() == false);
        REQUIRE(job != nullptr);
        REQUIRE(job->version == jobPool.version());

        job->func(job);

        REQUIRE(jobsRun == 1);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Pool Reset", "[core::job::jobPool]")
    {
        JobPool jobPool{ 1 };
        uint32_t jobsRun = 0;

        auto handle0 = jobPool.createJob(0, jobTest, &jobsRun);
        auto handle1 = jobPool.createJob(0, jobTest, &jobsRun);

        auto job0 = jobPool.resolve(handle0);
        auto job1 = jobPool.resolve(handle1);

        REQUIRE(job0->version == jobPool.version());
        REQUIRE(job1->version == jobPool.version());

        job0->func(job0);
        job1->func(job1);

        REQUIRE(jobsRun == 2);

        jobPool.sync();        // Clear current jobs, reset job pools, and increment version.

        REQUIRE(job0->version != jobPool.version());
        REQUIRE(job1->version != jobPool.version());

        // Create a third job. This job should use the same memory address as the first (now reset) job.
        // Though their addresses are the same, their versions should differ.
        auto handle2 = jobPool.createJob(0, jobTest, &jobsRun);
        auto job2 = jobPool.resolve(handle2);

        REQUIRE(job2->version == jobPool.version());
        REQUIRE(job2->version != job1->version);
        REQUIRE(job2 == job0);

        job2->func(job2);

        REQUIRE(jobsRun == 3);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Many Jobs", "[core::job::jobPool]")
    {
        // Allocate enough jobs to (a) exceed the number in the thread-local pools (1024 atm) and fill multiple pages of the global pool (1024 per global page atm)
        constexpr uint32_t jobsCount = JobPoolCount * 8;

        JobPool jobPool{ 1 };
        std::vector<JobHandle> handles;
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
            auto job = jobPool.resolve(handles[i]);
            job->func(job);
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
            auto job = jobPool.resolve(handles[i]);
            job->func(job);
        }

        auto endRun1 = std::chrono::steady_clock::now();
        auto timeToAllocate1 = (startRun1 - startAllocate1);
        auto timeToRun1 = (endRun1 - startRun1);

        REQUIRE(jobsRun == jobsCount * 2);
    } LITL_END_TEST_CASE
}