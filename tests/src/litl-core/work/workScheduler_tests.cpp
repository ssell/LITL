#include <catch2/catch_test_macros.hpp>
#include "litl-core/work/workScheduler.hpp"

namespace
{
    struct JobData
    {
        uint32_t* ptr;
    };

    /// <summary>
    /// Job using job->data, which is a void* to some shared address that is expected to be valid until at least the end of job execution.
    /// </summary>
    /// <param name="job"></param>
    void jobSharedDataTest(LITL::Core::Job* job)
    {
        auto* jobsRun = static_cast<uint32_t*>(job->data);
        (*jobsRun)++;
    }

    /// <summary>
    /// Job using job->localData, which is a copy of data provided when the job was created.
    /// </summary>
    /// <param name="job"></param>
    void jobLocalDataTest(LITL::Core::Job* job)
    {
        auto& jobData = job->getLocalData<JobData>();
        (*jobData.ptr)++;
    }
}

TEST_CASE("CreateAndSubmit SharedData", "[core::work::workScheduler]")
{
    LITL::Core::WorkScheduler scheduler;

    uint32_t jobsRun = 0;

    scheduler.createAndSubmit(jobSharedDataTest, &jobsRun);
    scheduler.wait();

    REQUIRE(jobsRun == 1);
}

TEST_CASE("CreateAndSubmit LocalData", "[core::work::workScheduler]")
{
    LITL::Core::WorkScheduler scheduler;

    uint32_t jobsRun = 0;
    JobData data{ &jobsRun };

    scheduler.createAndSubmit(jobLocalDataTest, data, nullptr);
    scheduler.wait();

    REQUIRE(jobsRun == 1);
}

TEST_CASE("Job Dependency Chain", "[core::work::workScheduler]")
{
    LITL::Core::WorkScheduler scheduler;

    uint32_t jobsRun = 0;

    auto handle0 = scheduler.create(jobSharedDataTest, &jobsRun);
    auto handle1 = scheduler.create(jobSharedDataTest, &jobsRun);
    auto handle2 = scheduler.create(jobSharedDataTest, &jobsRun);

    scheduler.addDependency(handle1, handle0);
    scheduler.addDependency(handle2, handle1);

    scheduler.submit(handle0);
    scheduler.wait();

    // handle0 runs which triggers handle1 which triggers handle2

    REQUIRE(jobsRun == 3);
}

TEST_CASE("Job Dependnecy Limit", "[core:;work::workScheduler]")
{
    LITL::Core::WorkScheduler scheduler;

    std::vector<LITL::Core::JobHandle> handles;
    uint32_t jobsRun = 0;

    handles.push_back(scheduler.create(jobSharedDataTest, &jobsRun));

    for (auto i = 0ul; i < LITL::Core::Job::JobMaxDependentsCount + 1; ++i)
    {
        handles.push_back(scheduler.create(jobSharedDataTest, &jobsRun));
        bool dependencyAdded = scheduler.addDependency(handles[i + 1], handles[0]);

        if (i < LITL::Core::Job::JobMaxDependentsCount)
        {
            REQUIRE(dependencyAdded == true);
        }
        else
        {
            REQUIRE(dependencyAdded == false);
        }
    }

    scheduler.submit(handles[0]);
    scheduler.wait();

    REQUIRE(jobsRun == (LITL::Core::Job::JobMaxDependentsCount + 1));
}