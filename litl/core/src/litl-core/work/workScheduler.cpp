#include <chrono>
#include <random>
#include <semaphore>
#include <thread>

#include "litl-core/alignment.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/math/math.hpp"
#include "litl-core/work/workDeque.hpp"
#include "litl-core/work/workScheduler.hpp"

namespace LITL::Core
{
    thread_local uint32_t WorkScheduler::t_threadIndex = std::numeric_limits<uint32_t>::max();

    struct WorkScheduler::Impl
    {
        /// <summary>
        /// The global and thread-specific job pools.
        /// </summary>
        JobPool jobPool;

        /// <summary>
        /// Number of jobs waiting or being processed.
        /// </summary>
        std::atomic<uint32_t> jobCount{ 0 };

        /// <summary>
        /// Is the scheduler currently running.
        /// This does not mean "are there jobs?" it means "we are not shutting down"
        /// </summary>
        std::atomic<bool> running{ true };

        /// <summary>
        /// No idea what this is.
        /// </summary>
        std::vector<std::unique_ptr<Worker>> workers;

        /// <summary>
        /// Signals when the scheduler is done processing all queued jobs.
        /// </summary>
        std::binary_semaphore busySignal{ 0 };
    };

    struct alignas(CacheLineSize) WorkScheduler::Worker
    {
        /// <summary>
        /// Collection of Jobs that are waiting to be executed.
        /// Each thread (via it's thread-specific Worker) has it's own deque of Jobs.
        /// </summary>
        WorkDeque deque;

        /// <summary>
        /// The thread that this Worker is running on.
        /// Reference is stored so that the Scheduler can join it on shutdown.
        /// </summary>
        std::thread thread;

        /// <summary>
        /// Worker uses this to wait/idle while there are no jobs to execute.
        /// </summary>
        std::binary_semaphore wake{ 0 };
    };

    WorkScheduler::WorkScheduler(uint32_t threadCount)
        : m_pImpl(std::make_unique<WorkScheduler::Impl>())
    {
        threadCount = Math::clamp((threadCount > 0 ? threadCount : std::thread::hardware_concurrency() - 1), 1ul, 32ul);

        m_pImpl->workers.resize(threadCount);

        for (uint32_t i = 0; i < threadCount; ++i)
        {
            m_pImpl->workers[i] = std::make_unique<Worker>();
            m_pImpl->workers[i]->thread = std::thread([this, i] { workerInternalLoop(i); });
        }
    }

    WorkScheduler::~WorkScheduler()
    {
        // Mark the scheduler as no longer running. The inner worker loop checks this on each iteration.
        m_pImpl->running.store(false, std::memory_order_release);

        for (auto& worker : m_pImpl->workers)
        {
            // Wake all idle workers.
            worker->wake.release();
        }

        for (auto& worker : m_pImpl->workers)
        {
            // Join all working workers.
            if (worker->thread.joinable())
            {
                worker->thread.join();
            }
        }

        // Once all jobs are done, clean up any lingering dead buffers.
        for (auto& worker : m_pImpl->workers)
        {
            worker->deque.clean();
        }
    }

    Job* WorkScheduler::create(Job::JobFunc func, void* externalData) noexcept
    {
        return m_pImpl->jobPool.createJob(t_threadIndex, func, externalData);
    }

    void WorkScheduler::createAndSubmit(Job::JobFunc func, void* externalData) noexcept
    {
        submit(create(func, externalData));
    }

    void WorkScheduler::submit(Job* job) const noexcept
    {
        // Push to the worker associated with this thread. If this is from an external thread then push to worker 0.
        const uint32_t workerIndex = (t_threadIndex < m_pImpl->workers.size() ? t_threadIndex : 0);

        if (m_pImpl->jobCount.fetch_add(1, std::memory_order_acq_rel) == 0)
        {
            // Scheduler was empty. Is no longer.
            m_pImpl->busySignal.acquire();
        }

        m_pImpl->workers[workerIndex]->deque.push(job);

        // Check all workers for an idle one.
        for (auto& worker : m_pImpl->workers)
        {
            if (worker->wake.try_acquire())
            {
                // Worker is idle, wake it up
                worker->wake.release();
                break;
            }
        }
    }

    bool WorkScheduler::addDependency(Job* dependent, Job* dependency) const noexcept
    {
        return m_pImpl->jobPool.addDependency(dependent, dependency);
    }

    void WorkScheduler::workerInternalLoop(uint32_t threadIndex)
    {
        t_threadIndex = threadIndex;
        auto& self = *(m_pImpl->workers[t_threadIndex]);
        std::minstd_rand prng(t_threadIndex);           // Low quality RNG, but fast.

        // While the scheduler is running ...
        while (m_pImpl->running.load(std::memory_order_relaxed))
        {
            auto job = self.deque.pop();

            if (!job.has_value())
            {
                // The deque for this worker's thread is empty.
                // Try to steal a job from another thread.
                const uint32_t victimIndex = prng() % m_pImpl->workers.size();

                if (victimIndex != t_threadIndex)
                {
                    job = m_pImpl->workers[victimIndex]->deque.steal();
                }
            }

            if (job.has_value())
            {
                // Run the job
                try
                {
                    (*job)->func((*job), t_threadIndex);

                    // Signal to any dependents that this job is completed
                    for (auto* dependent : (*job)->dependents)
                    {
                        // Decrease the dependent's dependency count and if this was the last dependency, submit it to the scheduler.
                        if (dependent->dependencyCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
                        {
                            submit(dependent);
                        }
                    }
                }
                catch (std::exception e)
                {
                    LITL_LOG_ERROR_CAPTURE("Caught unhandled exception running Job. Exception: ", e.what());
                }
                catch (...)
                {
                    LITL_LOG_ERROR_CAPTURE("Caught unhandled non-standard exception running Job.");
                }

                if (m_pImpl->jobCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
                {
                    // Scheduler is now out of jobs.
                    m_pImpl->busySignal.release();
                }

                // Worker will immediately return to the top of the loop and try to pull another job to execute.
            }
            else
            {
                // No jobs to be done. Wait and try again. The worker is now idle.
                auto _ = self.wake.try_acquire_for(std::chrono::microseconds(100));
            }
        }
    }

    uint32_t WorkScheduler::jobCount() const noexcept
    {
        return m_pImpl->jobCount.load();
    }

    uint32_t WorkScheduler::workerCount() const noexcept
    {
        return m_pImpl->workers.size();
    }

    void WorkScheduler::join() const noexcept
    {

    }
}