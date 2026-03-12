#include <atomic>
#include <chrono>
#include <semaphore>
#include <tuple>

#include "litl-core/math/traits.hpp"
#include "litl-core/job/jobFence.hpp"
#include "litl-core/job/jobScheduler.hpp"

namespace LITL::Core
{
    struct JobFence::Impl
    {
        JobPriority priority;
        std::atomic<int32_t> remaining{ 0 };
        std::binary_semaphore readySignal{ 0 };
    };

    JobFence::JobFence()
    {

    }

    JobFence::JobFence(std::span<JobHandle> jobHandles)
    {
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
        
        if (m_impl->remaining.fetch_add(1, std::memory_order_acq_rel) == 0)
        {
            // Set the fence priority to match that of the first job added.
            m_impl->priority = handle.job->priority;
        }
    }

    void JobFence::release(JobHandle handle) noexcept
    {
        if ((handle.job == nullptr) || (handle.job->fence != this))
        {
            return;
        }

        auto before = m_impl->remaining.fetch_sub(1, std::memory_order_acq_rel);
        auto after = m_impl->remaining.load();

        if (before == 1)
        {
            m_impl->readySignal.release();
        }
    }

    bool JobFence::wait(JobScheduler* scheduler, uint32_t timeoutMs) noexcept
    {
        const auto timeoutNs = static_cast<long long>(timeoutMs * Math::Constants::millisecond_to_nanoseconds);
        const auto start = std::chrono::steady_clock::now();

        bool timedOut = false;

        const auto remaining = m_impl->remaining.load();

        while (remaining > 0)
        {
            if ((timeoutMs > 0) && ((std::chrono::steady_clock::now() - start).count() > timeoutNs))
            {
                timedOut = true;
                break;
            }

            // Perform a productive wait and try to process a job on this thread that is waiting.
            // We pull only jobs that are the same priority level as the fence (and ideally as the jobs being fenced).
            // This is to prevent the fence from grabbing and blocking on a slower low priority background job
            // when all of it's fenced jobs are higher priority fast jobs.
            auto handle = scheduler->acquireJob(m_impl->priority);

            if (handle.has_value())
            {
                scheduler->run((*handle));
            }
            else
            {
                // No job to run (well, our steal failed). Wait and try again.
                std::ignore = m_impl->readySignal.try_acquire_for(std::chrono::microseconds(50));
            }
        }

        return timedOut;
    }
}