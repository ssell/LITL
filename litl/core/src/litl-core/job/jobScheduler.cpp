#include <barrier>
#include <chrono>
#include <memory>
#include <semaphore>
#include <thread>

#include "litl-core/constants.hpp"
#include "litl-core/thread.hpp"
#include "litl-core/math.hpp"
#include "litl-core/math/random.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/job/jobDeque.hpp"
#include "litl-core/job/jobFence.hpp"
#include "litl-core/job/jobScheduler.hpp"

namespace litl
{
    static constexpr uint32_t MainThreadIndex = 0;
    thread_local uint32_t JobScheduler::t_threadIndex = std::numeric_limits<uint32_t>::max();

    struct JobScheduler::Impl
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
        /// 
        /// Note that waiting is slightly different from syncing.
        /// While waiting, the thread that called JobScheduler::wait is performing
        /// useful work until the job count is 0.
        /// 
        /// Once the job count is 0, it moves onto syncing which is where
        /// the waiting thread is waiting for all in-progress Workers to complete.
        /// </summary>
        std::atomic<bool> waiting{ false };

        /// <summary>
        /// Used to halt workers when a sync is requested.
        /// A sync generation is used to support separate sync cycles.
        /// 
        /// While in production this should be uncommon (or impossible) it is 
        /// a situation that arises when performing bulk tests.
        /// </summary>
        std::atomic<uint32_t> syncGeneration{ 0 };

        /// <summary>
        /// The most recently completed sync generation cycle.
        /// </summary>
        std::atomic<uint32_t> syncCompleteGeneration{ 0 };

        /// <summary>
        /// Barrier used to synchronize all workers at a sync point.
        /// Sized to workerCount + 1 (main thread participates).
        /// </summary>
        std::unique_ptr<std::barrier<>> syncBarrier;

        /// <summary>
        /// No idea what this is.
        /// </summary>
        std::vector<std::unique_ptr<Worker>> workers;
    };

    struct alignas(Constants::cache_line_size) JobScheduler::Worker
    {
        /// <summary>
        /// Collection of Jobs that are waiting to be executed.
        /// Each thread (via it's thread-specific Worker) has one deque for each JobPriority level.
        /// </summary>
        std::array<JobDeque, static_cast<uint32_t>(JobPriority::__JobPriorityCount)> deques;

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

    JobScheduler::JobScheduler()
        : m_pImpl(std::make_unique<JobScheduler::Impl>())
    {
        // Work Scheduler needs to be created on the main thread so that this properly captures.
        t_threadIndex = MainThreadIndex;

        uint32_t threadCount = max(2u, std::thread::hardware_concurrency());  // - 1 (to prevent main thread being a dedicated worker, but then) + 1 (to have a dedicated worker for High priority jobs)

        m_pImpl->workers.resize(threadCount);
        m_pImpl->syncBarrier = std::make_unique<std::barrier<>>(threadCount);

        // First create all workers
        for (uint32_t i = 0; i < threadCount; ++i)
        {
            m_pImpl->workers[i] = std::make_unique<Worker>();
        }

        // Then launch their threads. If you do not wait to launch then you can crash as they try to steal from non-existent workers.
        // Skip worker[0] which is the main thread. That does not have its own dedicated workerInternalLoop running but instead is
        // reserved only for JobFence::wait and JobScheduler::wait calls from the main thread.
        for (uint32_t i = 1; i < threadCount; ++i)
        {
            m_pImpl->workers[i]->thread = std::thread([this, i] { workerInternalLoop(i); });
        }

        m_pImpl->workers.back()->dedicatedPriority = JobPriority::High;
    }

    JobScheduler::~JobScheduler()
    {
        // Mark the scheduler as no longer running. The inner worker loop checks this on each iteration.
        m_pImpl->running.store(false, std::memory_order_release);

        for (auto& worker : m_pImpl->workers)
        {
            // Wake all idle workers.
            std::ignore = worker->wake.try_acquire();  // drain if already released
            worker->wake.release();
        }

        // Skip index 0 (main thread)
        for (auto i = 1; i < m_pImpl->workers.size(); ++i)
        {
            // Join all working workers.
            if (m_pImpl->workers[i]->thread.joinable())
            {
                m_pImpl->workers[i]->thread.join();
            }
        }

        // Once all jobs are done, clean up any lingering dead buffers.
        for (auto& worker : m_pImpl->workers)
        {
            for (auto& priorityDeque : worker->deques)
            {
                priorityDeque.clean();
            }
        }
    }

    uint32_t JobScheduler::jobCount() const noexcept
    {
        return m_pImpl->jobCount.load();
    }

    uint32_t JobScheduler::workerCount() const noexcept
    {
        return m_pImpl->workers.size();
    }

    Job* JobScheduler::resolve(JobHandle handle) const noexcept
    {
        return m_pImpl->jobPool.resolve(handle);
    }

    bool JobScheduler::valid(JobHandle handle) const noexcept
    {
        auto job = resolve(handle);
        assert(job != nullptr);
        return (job->version == m_pImpl->jobPool.version());
    }

    JobHandle JobScheduler::create(Job::JobFunc func, void* externalData) noexcept
    {
        return m_pImpl->jobPool.createJob(t_threadIndex, func, externalData);
    }

    void JobScheduler::createAndSubmit(Job::JobFunc func, JobPriority priority, void* externalData) noexcept
    {
        submit(create(func, externalData), priority);
    }

    void JobScheduler::createAndSubmit(Job::JobFunc func, JobFence& fence, void* externalData) noexcept
    {
        submit(create(func, externalData), fence);
    }

    void JobScheduler::submit(JobHandle handle, JobFence& fence) const noexcept
    {
        fence.add(handle);
        return submit(handle, fence.priority());
    }

    void JobScheduler::submit(JobHandle handle, JobPriority priority) const noexcept
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
            logWarning("Job submitted during JobScheduler hard sync point. This should generally not happen.");
        }

        auto job = resolve(handle);

        assert(job != nullptr);

        job->priority = (static_cast<uint32_t>(priority) < static_cast<uint32_t>(JobPriority::__JobPriorityCount)) ? priority : JobPriority::Normal;
        job->state = JobState::Scheduled;

        // Push to the worker associated with this thread. If this is from an external thread then push to worker 0.
        const uint32_t workerIndex = (t_threadIndex < m_pImpl->workers.size() ? t_threadIndex : 0);

        std::ignore = m_pImpl->jobCount.fetch_add(1, std::memory_order_acq_rel);
        m_pImpl->workers[workerIndex]->deques[static_cast<uint32_t>(job->priority)].push(handle);

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

    bool JobScheduler::addDependency(JobHandle dependent, JobHandle dependency) const noexcept
    {
        return m_pImpl->jobPool.addDependency(dependent, dependency);
    }

    void JobScheduler::workerInternalLoop(uint32_t threadIndex) const
    {
        t_threadIndex = threadIndex;
        auto& self = *(m_pImpl->workers[t_threadIndex]);

        // While the scheduler is running ...
        while (m_pImpl->running.load(std::memory_order_relaxed))
        {
            const auto syncGeneration = m_pImpl->syncGeneration.load(std::memory_order_acquire);
            const auto completedSyncGeneration = m_pImpl->syncCompleteGeneration.load(std::memory_order_acquire);

            // Check if a sync has been requested for the current generation. If so, park at the barrier until the main thread has finished resetting the pool.
            if (syncGeneration > completedSyncGeneration)
            {
                // A sync is in progress that hasn't completed yet. Participate.
                m_pImpl->syncBarrier->arrive_and_wait();

                // Wait for THIS generation to complete.
                while ((m_pImpl->syncCompleteGeneration.load(std::memory_order_acquire) < syncGeneration) && m_pImpl->running.load(std::memory_order_relaxed))
                {
                    std::this_thread::yield();
                }

                continue;
            }

            std::optional<JobHandle> handle = std::nullopt;
            bool wasJobStolen = false;

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
                    wasJobStolen = handle.has_value();
                }
            }

            if (handle.has_value())
            {
                run((*handle), wasJobStolen);
            }
            else
            {
                // No jobs to be done. Wait and try again. The worker is now idle.
                std::ignore = self.wake.try_acquire_for(std::chrono::microseconds(50));
            }
        }
    }

    std::optional<JobHandle> JobScheduler::stealWork(JobPriority priority) const noexcept
    {
        // Try to steal a job from another thread.
        uint32_t victimIndex = FastRng::shared().next(m_pImpl->workers.size());

        if (victimIndex != t_threadIndex)
        {
            return m_pImpl->workers[victimIndex]->deques[static_cast<uint32_t>(priority)].steal();
        }

        return std::nullopt;
    }

    std::optional<JobHandle> JobScheduler::stealAnyWork() const noexcept
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

    std::optional<JobHandle> JobScheduler::acquireJob(JobPriority priority) const noexcept
    {
        // First try to pop if on a local worker thread.
        if (t_threadIndex < m_pImpl->workers.size())
        {
            auto jobHandle = (m_pImpl->workers[t_threadIndex])->deques[static_cast<uint32_t>(priority)].pop();

            if (jobHandle.has_value())
            {
                return jobHandle;
            }
        }

        // Otherwise try to steal.
        return stealWork(priority);
    }

    void JobScheduler::run(JobHandle handle, bool stolen) const noexcept
    {
        auto job = resolve(handle);

        assert(job->state == JobState::Scheduled);

        // Note we only check validty for executing the job function.
        // Validity does not matter for any of the following steps (dependencies and fences).
        // We _want_ to clear away dependencies and fences for invalid jobs to avoid deadlocks, etc.
        if (valid(handle))
        {
            job->state = (stolen ? JobState::RunningOnThief : JobState::RunningOnOwner);
            job->func(job);
        }

        job->state = JobState::Complete;

        // Signal to any dependents that this job is completed
        for (auto& dependent : job->dependents)
        {
            auto dependentJob = resolve(dependent);

            if (dependentJob == nullptr)
            {
                break;
            }

            // Decrease the dependent's dependency count and if this was the last dependency, submit it to the scheduler.
            if (dependentJob->dependencyCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                submit(dependent, job->priority);
            }
        }

        // Let the fence (if there is one) know that this job is complete.
        if (job->fence != nullptr)
        {
            job->fence->release(handle);
        }

        std::ignore = m_pImpl->jobCount.fetch_sub(1, std::memory_order_acq_rel);
    }

    bool JobScheduler::wait(uint32_t timeoutMs) const noexcept
    {
        ThreadSpin spinner;

        const auto start = std::chrono::steady_clock::now();
        const auto timeoutNs = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(timeoutMs));
        bool timedOut = false;

        m_pImpl->waiting = true;

        // While there are pending jobs, be productive and try to do some work (same thing essentially as our JobFence) 
        while (m_pImpl->jobCount > 0)
        {
            std::optional<JobHandle> handle = std::nullopt;

            for (auto i = 0; i < static_cast<uint32_t>(JobPriority::__JobPriorityCount) && !handle.has_value(); ++i)
            {
                handle = acquireJob(static_cast<JobPriority>(i));
            }

            if (handle.has_value())
            {
                run((*handle), true);
                spinner.reset();
            }
            else
            {
                spinner.spin();
            }

            if ((timeoutMs > 0) && ((std::chrono::steady_clock::now() - start) >= timeoutNs))
            {
                timedOut = true;
                break;
            }
        }

        if (!timedOut)
        {
            // Advance the generation to signal a new sync.
            auto syncGeneration = m_pImpl->syncGeneration.fetch_add(1, std::memory_order_acq_rel) + 1;

            // Wake any workers that are sleeping so they see the syncing flag.
            for (auto& worker : m_pImpl->workers)
            {
                std::ignore = worker->wake.try_acquire();
                worker->wake.release();
            }

            // Wait for every worker to reach the barrier.
            // At this point no worker is inside run() or holding a live handle.
            m_pImpl->syncBarrier->arrive_and_wait();

            // All workers are parked at the barrier now. Reset the pool and deques.
            m_pImpl->jobPool.sync();
            
            for (auto& worker : m_pImpl->workers)
            {
                for (auto& priorityDeque : worker->deques)
                {
                    priorityDeque.clean();
                }
            }

            // Signal that this generation's sync is complete.
            m_pImpl->syncCompleteGeneration.store(syncGeneration, std::memory_order_release);
        }

        m_pImpl->waiting = false;

        return !timedOut;
    }
}