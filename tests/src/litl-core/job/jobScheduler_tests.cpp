#include <atomic>
#include "tests.hpp"
#include "litl-core/math.hpp"
#include "litl-core/job/jobFence.hpp"
#include "litl-core/job/jobScheduler.hpp"

namespace litl::tests
{
    namespace
    {
        struct JobData
        {
            std::atomic<uint32_t> runs;
        };

        struct SharedJobData
        {
            std::atomic<uint32_t>* ptr;
        };

        /// <summary>
        /// Job using job->data, which is a void* to some shared address that is expected to be valid until at least the end of job execution.
        /// </summary>
        /// <param name="job"></param>
        void jobSharedDataTest(Job* job)
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

    LITL_TEST_CASE("CreateAndSubmit SharedData", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;
        std::atomic<uint32_t> jobsRun{ 0 };

        scheduler.createAndSubmit(jobSharedDataTest, JobPriority::Normal, &jobsRun);

        REQUIRE(scheduler.wait() == true);
        REQUIRE(jobsRun == 1);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("CreateAndSubmit LocalData", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;
        std::atomic<uint32_t> jobsRun{ 0 };
        SharedJobData data{ &jobsRun };   // atomics are not copyable or movable. but a pointer is

        scheduler.createAndSubmit(jobLocalDataTest, JobPriority::Normal, data, nullptr);
        REQUIRE(scheduler.wait() == true);
        REQUIRE(jobsRun == 1);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("CreateAndSubmit Lambda", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;
        JobData data{ 0 };

        scheduler.createAndSubmit([&data](Job* job) { data.runs++; }, JobPriority::Normal, nullptr);

        REQUIRE(scheduler.wait() == true);
        REQUIRE(data.runs == 1);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Job Dependency Chain", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;
        std::atomic<uint32_t> jobsRun{ 0 };

        auto handle0 = scheduler.create(jobSharedDataTest, &jobsRun);
        auto handle1 = scheduler.create(jobSharedDataTest, &jobsRun);
        auto handle2 = scheduler.create(jobSharedDataTest, &jobsRun);

        scheduler.addDependency(handle1, handle0);
        scheduler.addDependency(handle2, handle1);

        scheduler.submit(handle0, JobPriority::Normal);

        REQUIRE(scheduler.wait() == true);
        REQUIRE(jobsRun == 3);      // handle0 runs which triggers handle1 which triggers handle2
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Job Dependency Limit", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;

        std::vector<JobHandle> handles;
        std::atomic<uint32_t> jobsRun{ 0 };

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

        scheduler.submit(handles[0], JobPriority::Normal);

        REQUIRE(scheduler.wait() == true);
        REQUIRE(jobsRun == (Job::JobMaxDependentsCount + 1));       // the original job and its max number of dependents
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Job Handle Validity", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;
        std::atomic<uint32_t> jobsRun{ 0 };

        auto handle0 = scheduler.create(jobSharedDataTest, &jobsRun);

        REQUIRE(scheduler.valid(handle0) == true);

        scheduler.submit(handle0, JobPriority::Normal);

        while (scheduler.jobCount() > 0) {};

        REQUIRE(scheduler.valid(handle0) == true);
        REQUIRE(scheduler.wait() == true);
        REQUIRE(scheduler.valid(handle0) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Schedule Many Jobs", "[core::job::jobScheduler]")
    {
        constexpr uint32_t jobCount = 8192;

        JobScheduler scheduler;
        JobHandle handles[jobCount];
        std::atomic<uint32_t> jobsRun{ 0 };

        for (auto i = 0; i < jobCount; ++i)
        {
            handles[i] = scheduler.create(jobSharedDataTest, &jobsRun);
        }

        for (auto i = 0; i < jobCount; ++i)
        {
            scheduler.submit(handles[i], JobPriority::Normal);
        }

        REQUIRE(scheduler.wait() == true);
        REQUIRE(jobsRun == jobCount);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Schedule Many Jobs Priority", "[core::job::jobScheduler]")
    {
        // A sizeable total job count that is evenly divisible by the number of priority levels
        uint32_t jobCount = litl::pow(JobPriorityCount, 8);

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
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Fence", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;
        JobFence fence{ &scheduler };

        constexpr uint32_t jobCount = 1024;

        std::array<JobHandle, jobCount> handles;
        std::atomic<uint32_t> jobsRun = 0;

        for (auto i = 0; i < jobCount; ++i)
        {
            handles[i] = scheduler.create(jobSharedDataTest, &jobsRun);
            fence.add(handles[i]);
        }

        REQUIRE(scheduler.resolve(handles[0])->fence == &fence);

        for (auto i = 0; i < jobCount; ++i)
        {
            scheduler.submit(handles[i], JobPriority::Normal);
        }

        REQUIRE(fence.wait() == true);
        REQUIRE(jobsRun == jobCount);
        REQUIRE(scheduler.wait() == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Wait Multi-Fence Loop", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;

        constexpr uint32_t jobCount = 8192;
        
        for (auto i = 0; i < 100; ++i)
        {
            std::atomic<uint32_t> jobsRun{ 0 };
            JobFence fence0{ &scheduler, JobPriority::High };
            JobFence fence1{ &scheduler, JobPriority::Low };

            for (auto j = 0; j < jobCount; ++j)
            {
                if ((j % 2) == 0)
                {
                    scheduler.createAndSubmit(jobSharedDataTest, fence0, &jobsRun);
                }
                else
                {
                    scheduler.createAndSubmit(jobSharedDataTest, fence1, &jobsRun);
                }
            }

            REQUIRE(fence0.wait() == true);
            REQUIRE(fence1.wait() == true);
            REQUIRE(scheduler.wait() == true);
            REQUIRE(jobsRun == jobCount);
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Create Submit All Variants", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;
        JobFence fence{ &scheduler };
        std::atomic<uint32_t> jobsRun_shared{ 0 };
        SharedJobData jobsRun_localCopyShared{ &jobsRun_shared };   // atomics are not copyable or movable. but a pointer is

        // create using: function pointer + optional shared data
        // create(Job::JobFunc func, void* externalData)
        auto handle0 = scheduler.create(jobSharedDataTest, &jobsRun_shared);
        scheduler.submit(handle0, JobPriority::Normal);

        
        // create using: function pointer + local data copy + optional shared data
        // create(Job::JobFunc func, T& jobLocalData, void* externalData)
        auto handle1 = scheduler.create(jobLocalDataTest, jobsRun_localCopyShared, nullptr);
        scheduler.submit(handle1, JobPriority::Normal);

        // create and submit using: function pointer + priority level + optional shared data
        // createAndSubmit(Job::JobFunc func, JobPriority priority, void* externalData)
        scheduler.createAndSubmit(jobSharedDataTest, JobPriority::Normal, &jobsRun_shared);

        // create and submit using: function pointer + fence + optional shared data
        // createAndSubmit(Job::JobFunc func, JobFence& fence, void* externalData)
        scheduler.createAndSubmit(jobSharedDataTest, fence, &jobsRun_shared);

        // create and submit using: function pointer + priority + local data copy + optional shared data
        // createAndSubmit(Job::JobFunc func, JobPriority priority, T& jobLocalData, void* externalData)
        scheduler.createAndSubmit(jobLocalDataTest, JobPriority::Normal, jobsRun_localCopyShared, nullptr);

        // create and submit using: function pointer + fence + local data copy + optional shared data
        // createAndSubmit(Job::JobFunc func, JobFence& fence, T& jobLocalData, void* externalData)
        scheduler.createAndSubmit(jobLocalDataTest, fence, jobsRun_localCopyShared, nullptr);

        // create using: lambda + optional shared data
        // create(F&& func, void* externalData)
        auto handle2 = scheduler.create_lambda([](Job* job) {
                static_cast<std::atomic<uint32_t>*>(job->data)->fetch_add(1); 
            }, &jobsRun_shared);
        scheduler.submit(handle2, JobPriority::Normal);

        // create and submit using: lambda + priority + optional shared data
        // createAndSubmit(F&& func, JobPriority priority, void* externalData)
        scheduler.createAndSubmit([](Job* job) { 
                static_cast<std::atomic<uint32_t>*>(job->data)->fetch_add(1); 
            }, JobPriority::Normal, &jobsRun_shared);

        // create and submit using: lambda + fence + optional shared data
        // createAndSubmit(F&& func, JobFence& fence, void* externalData)
        scheduler.createAndSubmit([](Job* job) { 
                static_cast<std::atomic<uint32_t>*>(job->data)->fetch_add(1); 
            }, fence, &jobsRun_shared);

        REQUIRE(fence.wait() == true);
        REQUIRE(scheduler.wait() == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Job State", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;
        std::atomic<uint32_t> jobsRun_shared{ 0 };

        auto handle0 = scheduler.create(jobSharedDataTest, &jobsRun_shared);
        auto job0 = scheduler.resolve(handle0);

        REQUIRE(job0->version == 0);
        REQUIRE(job0->state == JobState::Idle);

        scheduler.submit(handle0, JobPriority::High);
        // can't reliably test the transition of idle -> scheduled -> running due to timing

        REQUIRE(scheduler.wait() == true);
        REQUIRE(job0->state == JobState::Complete);

        // ensure it is reset when reused
        handle0 = scheduler.create(jobSharedDataTest, &jobsRun_shared);
        REQUIRE(job0->version == 1);
        REQUIRE(job0->state == JobState::Idle);
    } LITL_END_TEST_CASE

    namespace
    {
        using SelfContainedJobIndirectFunc = void(uint32_t*, uint32_t);

        void selfContainedJobIndirectRun(uint32_t* dataPtr, uint32_t data)
        {
            (*dataPtr) += data;
        }

        struct SelfContainedJob
        {
            uint32_t dataBuffer[128];               // make the job struct large, to ensure we can capture even a large struct reference in the lambda closure.
            uint32_t data{ 0 };
            uint32_t* dataPtr{ nullptr };

            void run(Job* job)
            {
                (*dataPtr) += data;
            }

            void runIndirect(Job* job)
            {
                auto func = reinterpret_cast<SelfContainedJobIndirectFunc*>(job->data);
                func(dataPtr, data);
            }
        };
    }

    LITL_TEST_CASE("SelfContainedJob", "[core::job::jobScheduler]")
    {
        JobScheduler scheduler;
        uint32_t sharedData = 0;

        SelfContainedJob job{ {}, 5, &sharedData };

        scheduler.createAndSubmit([&job](Job* j)
            {
                job.run(j);
            }, JobPriority::Normal, nullptr);

        REQUIRE(scheduler.wait() == true);
        REQUIRE(sharedData == 5);

        job.data = 7;

        scheduler.createAndSubmit([&job](Job* j)
            {
                job.runIndirect(j);
            }, JobPriority::Normal, selfContainedJobIndirectRun);

        REQUIRE(scheduler.wait() == true);
        REQUIRE(sharedData == 12);

    } LITL_END_TEST_CASE;
}