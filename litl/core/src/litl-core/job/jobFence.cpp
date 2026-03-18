#include <atomic>
#include <chrono>
#include <tuple>

#include "litl-core/thread.hpp"
#include "litl-core/job/jobFence.hpp"
#include "litl-core/job/jobScheduler.hpp"

namespace LITL::Core
{
    struct JobFence::Impl
    {
        JobScheduler* scheduler;
        JobPriority priority;
        std::atomic<int32_t> remaining{ 0 };
    };

    JobFence::JobFence(JobScheduler* scheduler, JobPriority priority)
    {
        m_impl->scheduler = scheduler;
        m_impl->priority = priority;
    }

    JobFence::JobFence(JobScheduler* scheduler, std::span<JobHandle> jobHandles, JobPriority priority)
    {
        m_impl->scheduler = scheduler;
        m_impl->priority = priority;

        for (auto& handle : jobHandles)
        {
            add(handle);
        }
    }

    JobFence::~JobFence()
    {
        
    }

    void JobFence::add(JobHandle handle) noexcept
    {
        auto job = m_impl->scheduler->resolve(handle);

        if ((job == nullptr) || (job->fence != nullptr))
        {
            return;
        }

        job->fence = this;
        std::ignore = m_impl->remaining.fetch_add(1, std::memory_order_acq_rel);
    }

    void JobFence::release(JobHandle handle) noexcept
    {
        auto job = m_impl->scheduler->resolve(handle);

        if ((job == nullptr) || (job->fence != this))
        {
            return;
        }

        std::ignore = m_impl->remaining.fetch_sub(1, std::memory_order_acq_rel);
    }

    bool JobFence::wait(uint32_t timeoutMs) noexcept
    {
        ThreadSpin spinner;

        const auto timeout = std::chrono::milliseconds(timeoutMs);
        const auto start = std::chrono::steady_clock::now();
        auto timedOut = false;

        while (m_impl->remaining > 0)
        {
            // Perform a productive wait and try to process a job on this thread that is waiting.
            // We pull only jobs that are the same priority level as the fence (and ideally as the jobs being fenced).
            // This is to prevent the fence from grabbing and blocking on a slower low priority background job
            // when all of it's fenced jobs are higher priority fast jobs.
            auto handle = m_impl->scheduler->acquireJob(m_impl->priority);

            if (handle.has_value())
            {
                m_impl->scheduler->run((*handle), true);
                spinner.reset();
            }
            else
            {
                spinner.spin();
            }

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);

            if ((timeoutMs > 0) && (elapsed > timeout))
            {
                timedOut = true;
                break;
            }
        }

        return !timedOut;
    }

    JobPriority JobFence::priority() const noexcept
    {
        return m_impl->priority;
    }
}