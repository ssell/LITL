#include <chrono>
#include <random>
#include <semaphore>
#include <thread>
#include <tuple>

#include "litl-core/alignment.hpp"
#include "litl-core/thread.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/math/math.hpp"
#include "litl-core/work/workDeque.hpp"
#include "litl-core/work/workFence.hpp"
#include "litl-core/work/workScheduler.hpp"

namespace LITL::Core
{
    static constexpr uint32_t MainThreadIndex = 0;
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

        /// <summary>
        /// Default PRNG for non-Worker processed jobs.
        /// </summary>
        std::minstd_rand prng;

        Impl()
            : prng(std::chrono::system_clock::now().time_since_epoch().count())
        {

        }
    };

    struct alignas(CacheLineSize) WorkScheduler::Worker
    {
        /// <summary>
        /// Collection of Jobs that are waiting to be executed.
        /// Each thread (via it's thread-specific Worker) has one deque for each JobPriority level.
        /// </summary>
        std::array<WorkDeque, static_cast<uint32_t>(JobPriority::__JobPriorityCount)> deque;

        /// <summary>
        /// The scheduler creates one additional dedicated Worker for each priority level.
        /// 
        /// This is to avoid situations where all workers are tied up with slower low priority work 
        /// and there is critical work to be done. In this way there is always at least one worker
        /// available to process high priority jobs.
        /// </summary>
        std::optional<JobPriority> dedicatedPriority;

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

    WorkScheduler::WorkScheduler()
        : m_pImpl(std::make_unique<WorkScheduler::Impl>())
    {
        // Work Scheduler needs to be created on the main thread so that this properly captures.
        t_threadIndex = MainThreadIndex;

        uint32_t threadCount = std::thread::hardware_concurrency();  // - 1 (to prevent main thread being a dedicated worker, but then) + 1 (to have a dedicated worker for High priority jobs)

        m_pImpl->workers.resize(threadCount);

        for (uint32_t i = 0; i < threadCount; ++i)
        {
            m_pImpl->workers[i] = std::make_unique<Worker>();
            m_pImpl->workers[i]->thread = std::thread([this, i] { workerInternalLoop(i); });
        }

        m_pImpl->workers.back()->dedicatedPriority = JobPriority::High;
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
            for (auto i = 0ul; i < static_cast<uint32_t>(JobPriority::__JobPriorityCount); ++i)
            {
                worker->deque[i].clean();
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

    Job* WorkScheduler::create(Job::JobFunc func, void* externalData) noexcept
    {
        return m_pImpl->jobPool.createJob(t_threadIndex, func, externalData);
    }

    void WorkScheduler::createAndSubmit(Job::JobFunc func, void* externalData, JobPriority priority) noexcept
    {
        submit(create(func, externalData), priority);
    }

    void WorkScheduler::submit(Job* job, JobPriority priority) const noexcept
    {
        job->priority = (static_cast<uint32_t>(priority) < static_cast<uint32_t>(JobPriority::__JobPriorityCount)) ? priority : JobPriority::Normal;

        // Push to the worker associated with this thread. If this is from an external thread then push to worker 0.
        const uint32_t workerIndex = (t_threadIndex < m_pImpl->workers.size() ? t_threadIndex : 0);

        if (m_pImpl->jobCount.fetch_add(1, std::memory_order_acq_rel) == 0)
        {
            // Scheduler was empty. Is no longer.
            m_pImpl->busySignal.acquire();
        }

        m_pImpl->workers[workerIndex]->deque[static_cast<uint32_t>(job->priority)].push(job);

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

    void WorkScheduler::workerInternalLoop(uint32_t threadIndex) const
    {
        t_threadIndex = threadIndex;
        auto& self = *(m_pImpl->workers[t_threadIndex]);
        std::minstd_rand prng(t_threadIndex);           // Low quality RNG, but fast.

        // While the scheduler is running ...
        while (m_pImpl->running.load(std::memory_order_relaxed))
        {
            std::optional<Job*> job = std::nullopt;



            // Iterate through the priority levels: High -> Normal -> Low
            // Try get a local High priority job, then try to steal a High priority job.
            // Then try to get a local Normal priority job, then try to steal a Normal priority job.
            // Finally try to get a local Low priority job, then try to steal a Low priority job.
            for (auto i = 0ul; i < static_cast<uint32_t>(JobPriority::__JobPriorityCount) && !job.has_value(); ++i)
            {
                if (self.dedicatedPriority.has_value() && (self.dedicatedPriority.value() != static_cast<JobPriority>(i)))
                {
                    // This worker is reserved for only a specific priority level (likely High). So only process Jobs with matching priority.
                    continue;
                }

                job = self.deque[i].pop();

                if (!job.has_value())
                {
                    job = stealWork(prng, static_cast<JobPriority>(i));
                }
            }

            if (job.has_value())
            {
                run((*job));
            }
            else
            {
                // No jobs to be done. Wait and try again. The worker is now idle.
                std::ignore = self.wake.try_acquire_for(std::chrono::microseconds(50));
            }
        }
    }

    std::optional<Job*> WorkScheduler::stealWork(std::minstd_rand& prng, JobPriority priority) const noexcept
    {
        // Try to steal a job from another thread.
        const uint32_t victimIndex = prng() % m_pImpl->workers.size();

        if (victimIndex != t_threadIndex)
        {
            return m_pImpl->workers[victimIndex]->deque[static_cast<uint32_t>(priority)].steal();
        }

        return std::nullopt;
    }

    std::optional<Job*> WorkScheduler::acquireJob(JobPriority priority) const noexcept
    {
        return stealWork(m_pImpl->prng, priority);
    }

    void WorkScheduler::run(Job* job) const noexcept
    {
        // Run the job
        try
        {
            job->func(job, t_threadIndex);
        }
        catch (std::exception e)
        {
            LITL_LOG_ERROR_CAPTURE("Caught unhandled exception running Job. Exception: ", e.what());
        }
        catch (...)
        {
            LITL_LOG_ERROR_CAPTURE("Caught unhandled non-standard exception running Job.");
        }

        // Signal to any dependents that this job is completed
        for (auto* dependent : job->dependents)
        {
            // Decrease the dependent's dependency count and if this was the last dependency, submit it to the scheduler.
            if (dependent->dependencyCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                submit(dependent);
            }
        }

        // Let the fence (if there is one) know that this job is complete.
        if (job->fence != nullptr)
        {
            job->fence->release(job);
        }

        if (m_pImpl->jobCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            // Scheduler is now out of jobs.
            m_pImpl->busySignal.release();
        }
    }
}