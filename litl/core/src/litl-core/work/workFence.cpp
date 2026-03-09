#include <atomic>
#include <chrono>
#include <semaphore>
#include <tuple>

#include "litl-core/math/traits.hpp"
#include "litl-core/work/workFence.hpp"
#include "litl-core/work/workScheduler.hpp"

namespace LITL::Core
{
    WorkFence::WorkFence()
        : m_remaining(0)
    {

    }

    WorkFence::WorkFence(std::span<JobHandle> jobHandles)
        : m_remaining(0)
    {
        for (auto& handle : jobHandles)
        {
            add(handle);
        }
    }

    void WorkFence::add(JobHandle handle) noexcept
    {
        if ((handle.job == nullptr) || (handle.job->fence != nullptr))
        {
            return;
        }

        handle.job->fence = this;
        
        if (m_remaining.fetch_add(1, std::memory_order_acq_rel) == 0)
        {
            // Set the fence priority to match that of the first job added.
            m_priority = handle.job->priority;
        }
    }

    void WorkFence::release(JobHandle handle) noexcept
    {
        if ((handle.job == nullptr) || (handle.job->fence != this))
        {
            return;
        }

        if (m_remaining.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            m_readySignal.release();
        }
    }

    bool WorkFence::wait(WorkScheduler* scheduler, uint32_t timeoutMs) noexcept
    {
        const auto timeoutNs = static_cast<long long>(timeoutMs * Math::Constants::millisecond_to_nanoseconds);
        const auto start = std::chrono::steady_clock::now();

        bool timedOut = false;

        while (m_remaining.load(std::memory_order_acquire) > 0)
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
            auto handle = scheduler->acquireJob(m_priority);

            if (handle.has_value())
            {
                scheduler->run((*handle));
            }
            else
            {
                // No job to run (well, our steal failed). Wait and try again.
                std::ignore = m_readySignal.try_acquire_for(std::chrono::microseconds(50));
            }
        }

        return timedOut;
    }
}