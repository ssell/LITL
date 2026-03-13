#include <catch2/catch_test_macros.hpp>

#include "litl-core/math/math.hpp"
#include "litl-core/job/jobFence.hpp"
#include "litl-core/job/jobScheduler.hpp"

namespace LITL::Core::Tests
{
    namespace
    {
        struct JobData
        {
            uint32_t runs;
        };

        struct SharedJobData
        {
            uint32_t* ptr;
        };

        /// <summary>
        /// Job using job->data, which is a void* to some shared address that is expected to be valid until at least the end of job execution.
        /// </summary>
        /// <param name="job"></param>
        void jobSharedDataTest(Job* job)
        {
            auto* jobsRun = static_cast<uint32_t*>(job->data);
            (*jobsRun)++;
        }

        /// <summary>
        /// Job using a shared atomic counter.
        /// </summary>
        /// <param name="job"></param>
        void jobSharedAtomicDataTest(Job* job)
        {
            auto* jobsRun = static_cast<std::atomic<uint32_t>*>(job->data);
            jobsRun->fetch_add(1);
        }

        void jobSharedAtomicPriorityDataTest(Job* job)
        {
            auto* jobsRunByPriority = static_cast<std::array<std::atomic<uint32_t>, JobPriorityCount>*>(job->data);
            std::ignore = jobsRunByPriority->at(static_cast<uint32_t>(job->priority)).fetch_add(1);
        }

        /// <summary>
        /// Job using job->localData, which is a copy of data provided when the job was created.
        /// </summary>
        /// <param name="job"></param>
        void jobLocalDataTest(Job* job)
        {
            auto& jobData = job->getLocalData<SharedJobData>();
            (*jobData.ptr)++;
        }
    }

    TEST_CASE("CreateAndSubmit SharedData", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;

        uint32_t jobsRun = 0;

        scheduler.createAndSubmit(jobSharedDataTest, &jobsRun);

        REQUIRE(scheduler.wait() == true);
        REQUIRE(jobsRun == 1);
    }

    TEST_CASE("CreateAndSubmit LocalData", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;

        uint32_t jobsRun = 0;
        SharedJobData data{ &jobsRun };

        scheduler.createAndSubmit(jobLocalDataTest, data, nullptr);
        REQUIRE(scheduler.wait() == true);
        REQUIRE(jobsRun == 1);
    }

    TEST_CASE("CreateAndSubmit Lambda", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;
        JobData data{ 0 };

        scheduler.createAndSubmit([&data]() { data.runs++; }, nullptr);

        REQUIRE(scheduler.wait() == true);
        REQUIRE(data.runs == 1);
    }

    TEST_CASE("Job Dependency Chain", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;

        uint32_t jobsRun = 0;

        auto handle0 = scheduler.create(jobSharedDataTest, &jobsRun);
        auto handle1 = scheduler.create(jobSharedDataTest, &jobsRun);
        auto handle2 = scheduler.create(jobSharedDataTest, &jobsRun);

        scheduler.addDependency(handle1, handle0);
        scheduler.addDependency(handle2, handle1);

        scheduler.submit(handle0);

        REQUIRE(scheduler.wait() == true);
        REQUIRE(jobsRun == 3);      // handle0 runs which triggers handle1 which triggers handle2
    }

    TEST_CASE("Job Dependency Limit", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;

        std::vector<JobHandle> handles;
        uint32_t jobsRun = 0;

        handles.push_back(scheduler.create(jobSharedDataTest, &jobsRun));

        for (auto i = 0ul; i < Job::JobMaxDependentsCount + 1; ++i)
        {
            handles.push_back(scheduler.create(jobSharedDataTest, &jobsRun));
            bool dependencyAdded = scheduler.addDependency(handles[i + 1], handles[0]);

            if (i < Job::JobMaxDependentsCount)
            {
                REQUIRE(dependencyAdded == true);
            }
            else
            {
                REQUIRE(dependencyAdded == false);
            }
        }

        scheduler.submit(handles[0]);

        REQUIRE(scheduler.wait() == true);
        REQUIRE(jobsRun == (Job::JobMaxDependentsCount + 1));       // the original job and its max number of dependents
    }

    TEST_CASE("Job Handle Validity", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;
        uint32_t jobsRun = 0;

        auto handle0 = scheduler.create(jobSharedDataTest, &jobsRun);

        REQUIRE(scheduler.valid(handle0) == true);

        scheduler.submit(handle0);

        while (scheduler.jobCount() > 0) {};

        REQUIRE(scheduler.valid(handle0) == true);
        REQUIRE(scheduler.wait() == true);
        REQUIRE(scheduler.valid(handle0) == false);
    }

    TEST_CASE("Schedule Many Jobs", "[core::job::jobScheduler]")
    {
        constexpr uint32_t jobCount = 8192;

        JobScheduler scheduler;
        JobHandle handles[jobCount];
        std::atomic<uint32_t> jobsRun{ 0 };

        for (auto i = 0; i < jobCount; ++i)
        {
            handles[i] = scheduler.create(jobSharedAtomicDataTest, &jobsRun);
        }

        for (auto i = 0; i < jobCount; ++i)
        {
            scheduler.submit(handles[i]);
        }

        REQUIRE(scheduler.wait() == true);
        REQUIRE(jobsRun == jobCount);
    }

    TEST_CASE("Schedule Many Jobs Priority", "[core::job::jobScheduler]")
    {
        // A sizeable total job count that is evenly divisible by the number of priority levels
        uint32_t jobCount = LITL::Math::pow(JobPriorityCount, 8);

        JobScheduler scheduler;
        std::vector<JobHandle> handles;
        handles.reserve(jobCount);

        std::array<std::atomic<uint32_t>, JobPriorityCount> priorities;

        for (auto i = 0; i < jobCount; ++i)
        {
            handles.push_back(scheduler.create(jobSharedAtomicPriorityDataTest, &priorities));
        }

        for (auto i = 0; i < jobCount; ++i)
        {
            scheduler.submit(handles[i], static_cast<JobPriority>(i % JobPriorityCount));
        }

        REQUIRE(scheduler.wait() == true);

        for (auto i = 0; i < JobPriorityCount; ++i)
        {
            // Each priority level should have been run the same number of times
            priorities[i] = jobCount / JobPriorityCount;
        }
    }

    TEST_CASE("Fence", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;
        JobFence fence;

        constexpr uint32_t jobCount = 1024;
        std::array<JobHandle, jobCount> handles;
        uint32_t jobsRun = 0;

        for (auto i = 0; i < jobCount; ++i)
        {
            handles[i] = scheduler.create(jobSharedDataTest, &jobsRun);
            fence.add(handles[i]);
        }

        REQUIRE(handles[0].job->fence == &fence);

        for (auto i = 0; i < jobCount; ++i)
        {
            scheduler.submit(handles[i]);
        }

        REQUIRE(scheduler.wait() == true);
        REQUIRE(jobsRun == jobCount);
    }
}