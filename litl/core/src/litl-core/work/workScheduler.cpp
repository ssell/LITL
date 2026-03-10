#include <chrono>
#include <semaphore>
#include <thread>
#include <tuple>

#include "litl-core/alignment.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/math/math.hpp"
#include "litl-core/math/random.hpp"
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
        /// Is the scheduler currently waiting for all jobs to finish?
        /// </summary>
        std::atomic<bool> waiting{ false };

        /// <summary>
        /// No idea what this is.
        /// </summary>
        std::vector<std::unique_ptr<Worker>> workers;

        /// <summary>
        /// Signals when the scheduler is done processing all queued jobs.
        /// </summary>
        std::binary_semaphore busySignal{ 1 };
    };

    struct alignas(CacheLineSize) WorkScheduler::Worker
    {
        /// <summary>
        /// Collection of Jobs that are waiting to be executed.
        /// Each thread (via it's thread-specific Worker) has one deque for each JobPriority level.
        /// </summary>
        std::array<WorkDeque, static_cast<uint32_t>(JobPriority::__JobPriorityCount)> deques;

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

        // First create all workers
        for (uint32_t i = 0; i < threadCount; ++i)
        {
            m_pImpl->workers[i] = std::make_unique<Worker>();
        }

        // Then launch their threads. If you do not wait to launch then you can crash as they try to steal from non-existent workers.
        for (uint32_t i = 0; i < threadCount; ++i)
        {
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
                worker->deques[i].clean();
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

    bool WorkScheduler::valid(JobHandle handle) const noexcept
    {
        return handle.valid(m_pImpl->jobPool.version());
    }

    JobHandle WorkScheduler::create(Job::JobFunc func, void* externalData) noexcept
    {
        return m_pImpl->jobPool.createJob(t_threadIndex, func, externalData);
    }

    void WorkScheduler::createAndSubmit(Job::JobFunc func, void* externalData, JobPriority priority) noexcept
    {
        submit(create(func, externalData), priority);
    }

    void WorkScheduler::submit(JobHandle handle, JobPriority priority) const noexcept
    {
        if (!m_pImpl->running.load(std::memory_order_relaxed))
        {
            // Scheduler is shutting down
            return;
        }

        if (m_pImpl->waiting.load(std::memory_order_relaxed))
        {
            // Scheduler is at a hard sync point. If a job is submitted at a scheduler sync point, then something is likely wrong.
            // Accept new jobs but emit a warning as we typically do not want to add new jobs while syncing.
            // That can lead either to a deadlock (where the scheduler can never exit syncing) if work is continually
            // added, or timing can be off and work may be submitted inbetween the last worker finishing it's final job
            // and the job pool being reset. This would immediately invalidate the newly submitted job.
            logWarning("Job submitted during WorkScheduler hard sync point. This should generally not happen.");
        }

        handle.job->priority = (static_cast<uint32_t>(priority) < static_cast<uint32_t>(JobPriority::__JobPriorityCount)) ? priority : JobPriority::Normal;

        // Push to the worker associated with this thread. If this is from an external thread then push to worker 0.
        const uint32_t workerIndex = (t_threadIndex < m_pImpl->workers.size() ? t_threadIndex : 0);

        if (m_pImpl->jobCount.fetch_add(1, std::memory_order_acq_rel) == 0)
        {
            // Scheduler was empty. Is no longer.
            m_pImpl->busySignal.acquire();
        }

        m_pImpl->workers[workerIndex]->deques[static_cast<uint32_t>(handle.job->priority)].push(handle);

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

    bool WorkScheduler::addDependency(JobHandle dependent, JobHandle dependency) const noexcept
    {
        return m_pImpl->jobPool.addDependency(dependent, dependency);
    }

    void WorkScheduler::workerInternalLoop(uint32_t threadIndex) const
    {
        t_threadIndex = threadIndex;
        auto& self = *(m_pImpl->workers[t_threadIndex]);

        // While the scheduler is running ...
        while (m_pImpl->running.load(std::memory_order_relaxed))
        {
            std::optional<JobHandle> handle = std::nullopt;

            // Iterate through the priority levels: High -> Normal -> Low
            // Try get a local High priority job, then try to steal a High priority job.
            // Then try to get a local Normal priority job, then try to steal a Normal priority job.
            // Finally try to get a local Low priority job, then try to steal a Low priority job.
            for (auto i = 0ul; i < static_cast<uint32_t>(JobPriority::__JobPriorityCount) && !handle.has_value(); ++i)
            {
                if (self.dedicatedPriority.has_value() && (self.dedicatedPriority.value() != static_cast<JobPriority>(i)))
                {
                    // This worker is reserved for only a specific priority level (likely High). So only process Jobs with matching priority.
                    continue;
                }

                handle = self.deques[i].pop();

                if (!handle.has_value())
                {
                    handle = stealWork(static_cast<JobPriority>(i));
                }
            }

            if (handle.has_value())
            {
                run((*handle));
            }
            else
            {
                // No jobs to be done. Wait and try again. The worker is now idle.
                std::ignore = self.wake.try_acquire_for(std::chrono::microseconds(50));
            }
        }
    }

    std::optional<JobHandle> WorkScheduler::stealWork(JobPriority priority) const noexcept
    {
        // Try to steal a job from another thread.
        const uint32_t victimIndex = Math::FastRng::shared().next() % m_pImpl->workers.size();

        if (victimIndex != t_threadIndex)
        {
            return m_pImpl->workers[victimIndex]->deques[static_cast<uint32_t>(priority)].steal();
        }

        return std::nullopt;
    }

    std::optional<JobHandle> WorkScheduler::stealAnyWork() const noexcept
    {
        std::optional<JobHandle> handle = std::nullopt;

        for (auto i = 0ul; i < static_cast<uint32_t>(JobPriority::__JobPriorityCount); ++i)
        {
            handle = stealWork(static_cast<JobPriority>(i));

            if (handle.has_value())
            {
                break;
            }
        }

        return handle;
    }

    std::optional<JobHandle> WorkScheduler::acquireJob(JobPriority priority) const noexcept
    {
        return stealWork(priority);
    }

    void WorkScheduler::run(JobHandle handle) const noexcept
    {
        try
        {
            // Note we only check validty for executing the job function.
            // Validity does not matter for any of the following steps (dependencies and fences).
            // We _want_ to clear away dependencies and fences for invalid jobs to avoid deadlocks, etc.
            if (valid(handle))
            {
                handle.job->func(handle.job, t_threadIndex);
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

        // Signal to any dependents that this job is completed
        for (auto& dependent : handle.job->dependents)
        {
            // Decrease the dependent's dependency count and if this was the last dependency, submit it to the scheduler.
            if ((dependent.job != nullptr) && (dependent.job->dependencyCount.fetch_sub(1, std::memory_order_acq_rel) == 1))
            {
                submit(dependent);
            }
        }

        // Let the fence (if there is one) know that this job is complete.
        if (handle.job->fence != nullptr)
        {
            handle.job->fence->release(handle);
        }

        if (m_pImpl->jobCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            // Scheduler is now out of jobs.
            m_pImpl->busySignal.release();
        }
    }

    void WorkScheduler::wait(uint32_t timeoutMs) const noexcept
    {
        const auto start = std::chrono::steady_clock::now();
        const auto timeoutNs = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(timeoutMs * Math::Constants::microsecond_to_nanoseconds));

        m_pImpl->waiting = true;

        // While there are pending jobs, be productive and try to do some work (same thing essentially as our WorkFence) 
        while (m_pImpl->jobCount > 0)
        {
            auto handle = stealAnyWork();

            if (handle.has_value())
            {
                run((*handle));
            }
            else
            {
                std::ignore = m_pImpl->busySignal.try_acquire_for(std::chrono::microseconds(50));
            }

            if ((std::chrono::steady_clock::now() - start) >= timeoutNs)
            {
                break;
            }
        }

        // Reset the pools and increment the version
        m_pImpl->jobPool.sync();

        m_pImpl->waiting = false;
    }
}