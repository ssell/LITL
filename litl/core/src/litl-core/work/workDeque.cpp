#include <cassert>
#include <vector>

#include "litl-core/work/workDeque.hpp"

namespace LITL::Core
{
    /// <summary>
    /// Internal ring buffer used by the work stealing queue.
    /// 
    /// When using this buffer, the top index can never decrement but the bottom index can.
    /// The top index increments on steals, while the bottom increments on push and decrements on pops.
    /// 
    /// As such, the top chases the bottom and can potentially surpass the bottom momentarily in the event of a
    /// race between a combination owner-pop and thief-steal operation occurring at the same time.
    /// 
    /// The size (not capacity) of the buffer is equal to (bottom index - top index). 
    /// </summary>
    struct WorkDeque::RingBuffer
    {
        RingBuffer(size_t capacity = 1024)
            : mask(capacity - 1), jobs(capacity)
        {

        }

        Job*& operator[](size_t index)
        {
            return jobs[index & mask];
        }

        size_t capacity() const noexcept
        {
            return jobs.size();
        }

        RingBuffer* grow(size_t bottom, size_t top)
        {
            // Note we intentionally do not delete here (or in the caller directly).
            // "old" ring buffers may still be in used by other threads so a direct
            // delete at this point is unsafe. Instead a custom garbage collector is used.
            auto* newBuffer = new RingBuffer(capacity() * 2);

            for (size_t i = bottom; i < top; ++i)
            {
                (*newBuffer)[i] = (*this)[i];
            }

            return newBuffer;
        }

        /// <summary>
        /// A mask is required as the bottom and top indices of the owning queue never decrement.
        /// Both the top and the bottom are perpetually incrementing, and it is the mask thar
        /// wraps their indices around to fit inside the current buffer space.
        /// 
        /// This also allows for no changes to the referenced indices when the buffer is grown
        /// as the same indices (and their ranges) are valid between buffer sizes.
        /// </summary>
        const size_t mask;
        std::vector<Job*> jobs;
    };

    WorkDeque::WorkDeque()
        : m_pBuffer(new RingBuffer())
    {

    }

    WorkDeque::~WorkDeque()
    {

    }

    void WorkDeque::push(Job* job) noexcept
    {
        /**
         * Called by owner only.
         * Stores the job at the bottom of the buffer and then increments the bottom index.
         * If the buffer is full, it is resized prior to storage.
         */

        assert(job != nullptr);

        // Fetch indices and underlying ring buffer
        auto bottomIndex = m_bottom.load(std::memory_order_relaxed);
        auto topIndex = m_top.load(std::memory_order_acquire);
        auto* ringBuffer = m_pBuffer.load(std::memory_order_relaxed);

        // Resize if needed
        if ((bottomIndex - topIndex) >= static_cast<int64_t>(ringBuffer->capacity()))
        {
            m_deadBuffers.push_back(ringBuffer);
            ringBuffer = ringBuffer->grow(bottomIndex, topIndex);
            m_pBuffer.store(ringBuffer, std::memory_order_release);
        }

        // Store the job at the bottom index and then increment it.
        (*ringBuffer)[bottomIndex] = job;
        std::atomic_thread_fence(std::memory_order_release);
        m_bottom.store(bottomIndex + 1, std::memory_order_relaxed);
    }

    std::optional<Job*> WorkDeque::pop() noexcept
    {
        /**
         * Called by owner only.
         * Decrements the bottom index and then reads the top index.
         * 
         * If bottom > top, there is definitely a job available.
         * 
         * If bottom == top, then there is exactly one item left and so a thief may be racing us for it. 
         * We CAS to take it, but if that fails then the thief got to it first and it is gone.
         */

        std::optional<Job*> job = nullptr;

        // Fetch the bottom index and decrement it, fetch the ring buffer, fetch the top index.
        auto bottomIndex = m_bottom.load(std::memory_order_relaxed) - 1;
        m_bottom.store(bottomIndex, std::memory_order_relaxed);
        auto* ringBuffer = m_pBuffer.load(std::memory_order_relaxed);

        // v this (along with the one in steal) ensure that the thief's read of top and the owner's decrement of bottom are ordered
        std::atomic_thread_fence(std::memory_order_seq_cst);

        auto topIndex = m_top.load(std::memory_order_relaxed);

        // If there is at least one item in the buffer
        if (topIndex <= bottomIndex)
        {
            job = (*ringBuffer)[bottomIndex];

            // If there is exactly one item in the buffer
            if (topIndex == bottomIndex)
            {
                // Attempt to take it. If this fails then a thief stole it first.
                if (!m_top.compare_exchange_strong(topIndex, topIndex + 1, std::memory_order_seq_cst, std::memory_order_relaxed))
                {
                    job = std::nullopt;
                }

                // Restore the bottom index
                m_bottom.store(bottomIndex + 1, std::memory_order_relaxed);
            }

            // ... job was sucessfully popped off. leave bottom decremented ...
        }
        else
        {
            // Restore the bottom index, there was nothing to take and the bottom doesnt need to be decremented.
            m_bottom.store(bottomIndex + 1, std::memory_order_relaxed);
        }

        return job;
    }

    std::optional<Job*> WorkDeque::steal() noexcept
    {
        /**
         * Called by thieves.
         * 
         * Reads the top index, then reads the bottom index.
         * If the deque is not empty it tries a CAS on the top to steal it.
         * If the CAS fails then another thief - or the owner - got to it first.
         */

        std::optional<Job*> job = std::nullopt;

        // Fetch the top and bottom indices
        auto topIndex = m_top.load(std::memory_order_acquire);

        // v this (along with the one in pop) ensure that the thief's read of top and the owner's decrement of bottom are ordered
        std::atomic_thread_fence(std::memory_order_seq_cst);

        auto bottomIndex = m_bottom.load(std::memory_order_acquire);

        // If there is more than one item in the buffer
        if (topIndex < bottomIndex)
        {
            auto* ringBuffer = m_pBuffer.load(std::memory_order_consume);
            job = (*ringBuffer)[topIndex];

            // Attempt to take the item
            if (!m_top.compare_exchange_strong(topIndex, topIndex + 1, std::memory_order_seq_cst, std::memory_order_relaxed))
            {
                job = std::nullopt;
            }
        }

        return job;
    }

    void WorkDeque::clean() noexcept
    {
        for (auto* buffer : m_deadBuffers)
        {
            delete buffer;
        }

        m_deadBuffers.clear();
    }
}