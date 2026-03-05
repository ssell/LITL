#ifndef LITL_CORE_WORK_FENCE_H__
#define LITL_CORE_WORK_FENCE_H__

#include <span>

#include "litl-core/impl.hpp"

namespace LITL::Core
{
    struct Job;
    class WorkScheduler;

    /// <summary>
    /// A synchronization fence for a batch of jobs.
    /// </summary>
    class WorkFence
    {
    public:

        WorkFence();
        explicit WorkFence(std::span<Job*> jobs);

        /// <summary>
        /// Adds a job to be tracked by the fence.
        /// The fence will block while waiting until this job (and any others added) is complete.
        /// </summary>
        /// <param name="job"></param>
        void add(Job* job) noexcept;

        /// <summary>
        /// Lets the fence know that the specified job is done.
        /// If this was the last job, any wait will be released.
        /// </summary>
        /// <param name="job"></param>
        void release(Job* job) noexcept;

        /// <summary>
        /// Blocks until all tracked jobs are complete.
        /// This will trigger the scheduler to process jobs while waiting to avoid deadlocks.
        /// </summary>
        /// <param name="scheduler"></param>
        /// <param name="timeoutMs"></param>
        /// <returns>True if done waiting without timing out. False if timed out.</returns>
        bool wait(WorkScheduler* scheduler, uint32_t timeoutMs = 0) noexcept;

    protected:

    private:

        std::atomic<int32_t> m_remaining;
        std::binary_semaphore m_readySignal{ 0 };
    };
}

#endif