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

    WorkFence::WorkFence(std::span<Job*> jobs)
        : m_remaining(0)
    {
        for (auto* job : jobs)
        {
            add(job);
        }
    }

    void WorkFence::add(Job* job) noexcept
    {
        if ((job == nullptr) || (job->fence != nullptr))
        {
            return;
        }

        job->fence = this;
        m_remaining.fetch_add(1, std::memory_order_acq_rel);
    }

    void WorkFence::release(Job* job) noexcept
    {
        if ((job == nullptr) || (job->fence != this))
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

            auto job = scheduler->acquireJob();

            if (job.has_value())
            {
                scheduler->run((*job));
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