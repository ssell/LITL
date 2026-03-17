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
        JobPriority priority;
        std::atomic<int32_t> remaining{ 0 };
    };

    JobFence::JobFence(JobPriority priority)
    {
        m_impl->priority = priority;
    }

    JobFence::JobFence(std::span<JobHandle> jobHandles, JobPriority priority)
    {
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
        if ((handle.job == nullptr) || (handle.job->fence != nullptr))
        {
            return;
        }

        handle.job->fence = this;
        std::ignore = m_impl->remaining.fetch_add(1, std::memory_order_acq_rel);
    }

    void JobFence::release(JobHandle handle) noexcept
    {
        if ((handle.job == nullptr) || (handle.job->fence != this))
        {
            return;
        }

        std::ignore = m_impl->remaining.fetch_sub(1, std::memory_order_acq_rel);
    }

    bool JobFence::wait(JobScheduler const& scheduler, uint32_t timeoutMs) noexcept
    {
        ThreadSpin spinner;

        const auto timeoutNs = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(timeoutMs));
        const auto start = std::chrono::steady_clock::now();
        auto timedOut = false;

        while (m_impl->remaining > 0)
        {
            // Perform a productive wait and try to process a job on this thread that is waiting.
            // We pull only jobs that are the same priority level as the fence (and ideally as the jobs being fenced).
            // This is to prevent the fence from grabbing and blocking on a slower low priority background job
            // when all of it's fenced jobs are higher priority fast jobs.
            auto handle = scheduler.acquireJob(m_impl->priority);

            if (handle.has_value())
            {
                scheduler.run((*handle));
                spinner.reset();
            }
            else
            {
                spinner.spin();
            }

            if ((timeoutMs > 0) && ((std::chrono::steady_clock::now() - start) > timeoutNs))
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