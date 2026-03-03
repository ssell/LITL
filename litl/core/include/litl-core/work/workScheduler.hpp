#ifndef LITL_CORE_WORK_SCHEDULER_H__
#define LITL_CORE_WORK_SCHEDULER_H__

#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

#include "litl-core/work/job.hpp"

namespace LITL::Core
{
    class WorkScheduler
    {
    public:

        explicit WorkScheduler(uint32_t threadCount = 0);
        WorkScheduler(WorkScheduler const&) = delete;
        WorkScheduler& operator=(WorkScheduler const&) = delete;
        ~WorkScheduler();

        void submit(Job* job) noexcept;

    protected:

    private:

        void workerInternalLoop(uint32_t threadIndex);

        struct Worker;
        static thread_local uint32_t t_workerIndex;

        std::atomic<bool> m_running{ true };
        std::vector<std::unique_ptr<Worker>> m_workers;
    };
}

#endif