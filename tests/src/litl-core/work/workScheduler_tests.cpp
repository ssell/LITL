#include <catch2/catch_test_macros.hpp>
#include "litl-core/work/workScheduler.hpp"

namespace
{
    void jobTest(LITL::Core::Job* job, uint32_t threadIndex)
    {
        auto* jobsRun = static_cast<uint32_t*>(job->data);
        (*jobsRun)++;
    }
}

TEST_CASE("CreateAndSubmit", "[core::work::workScheduler]")
{
    LITL::Core::WorkScheduler scheduler;

    uint32_t jobsRun = 0;

    scheduler.createAndSubmit(jobTest, &jobsRun);
    scheduler.wait();

    REQUIRE(jobsRun == 1);
}