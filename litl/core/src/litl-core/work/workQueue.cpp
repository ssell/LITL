#include <cassert>
#include <vector>

#include "litl-core/work/workQueue.hpp"

namespace LITL::Core
{
    struct WorkQueue::RingBuffer
    {
        RingBuffer(size_t capacity = 1024)
            : jobs(capacity)
        {

        }

        Job*& operator[](size_t index)
        {
            return jobs[index & mask];
        }

        size_t size() const noexcept
        {
            return jobs.size();
        }

        RingBuffer* grow(size_t bottom, size_t top)
        {
            // Note we intentionally do not delete here (or in the caller directly).
            // "old" ring buffers may still be in used by other threads so a direct
            // delete at this point is unsafe. Instead a custom garbage collector is used.
            auto* newBuffer = new RingBuffer(size() * 2);

            for (size_t i = bottom; i < top; ++i)
            {
                (*newBuffer)[i] = (*this)[i];
            }

            return newBuffer;
        }

        /// <summary>
        /// Used to wrap the top around the end of the buffer.
        /// </summary>
        size_t mask;
        std::vector<Job*> jobs;
    };

    WorkQueue::WorkQueue()
        : m_pBuffer(new RingBuffer())
    {

    }

    WorkQueue::~WorkQueue()
    {

    }

    void WorkQueue::push(Job* job) noexcept
    {
        assert(job != nullptr);

        auto bottomIndex = m_bottom.load(std::memory_order_relaxed);
        auto topIndex = m_top.load(std::memory_order_acquire);          // create sync start
        auto* ringBuffer = m_pBuffer.load(std::memory_order_relaxed);

        if ((bottomIndex - topIndex) >= static_cast<uint32_t>(ringBuffer->size()))
        {
            ringBuffer = ringBuffer->grow(bottomIndex, topIndex);
            m_pBuffer.store(ringBuffer, std::memory_order_release);
        }

        (*ringBuffer)[bottomIndex] = job;
        std::atomic_thread_fence(std::memory_order_release);
        m_bottom.store(bottomIndex + 1, std::memory_order_relaxed);
    }

    Job* WorkQueue::pop() noexcept
    {
        Job* job = nullptr;

        auto bottomIndex = m_bottom.load(std::memory_order_relaxed) - 1;
        m_bottom.store(bottomIndex, std::memory_order_relaxed);

        auto* ringBuffer = m_pBuffer.load(std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_seq_cst);

        auto topIndex = m_top.load(std::memory_order_relaxed);

        if (topIndex <= bottomIndex)
        {
            auto* job = (*ringBuffer)[bottomIndex];

            if (topIndex == bottomIndex)
            {
                if (!m_top.compare_exchange_strong(topIndex, topIndex + 1, std::memory_order_seq_cst, std::memory_order_relaxed))
                {
                    job = nullptr;
                }

                m_bottom.store(bottomIndex + 1, std::memory_order_relaxed);
            }
        }
        else
        {
            m_bottom.store(bottomIndex + 1, std::memory_order_relaxed);
        }

        return job;
    }

    Job* WorkQueue::steal() noexcept
    {
        Job* job = nullptr;

        auto topIndex = m_top.load(std::memory_order_acquire);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        auto bottomIndex = m_bottom.load(std::memory_order_acquire);

        if (topIndex < bottomIndex)
        {
            auto* ringBuffer = m_pBuffer.load(std::memory_order_consume);
            job = (*ringBuffer)[topIndex];

            if (!m_top.compare_exchange_strong(topIndex, topIndex + 1, std::memory_order_seq_cst, std::memory_order_relaxed))
            {
                job = nullptr;
            }
        }

        return job;
    }
}