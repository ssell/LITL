#ifndef LITL_CORE_WORK_DEQUE_H__
#define LITL_CORE_WORK_DEQUE_H__

#include <atomic>
#include <cstdint>
#include <optional>
#include <vector>

#include "litl-core/constants.hpp"
#include "litl-core/job/job.hpp"

namespace litl
{
    /// <summary>
    /// Implementation of a Chase-Lev work-stealing deque.
    /// 
    /// https://www.dre.vanderbilt.edu/~schmidt/PDF/work-stealing-dequeue.pdf
    /// 
    /// As the size of the queue is expected to be relatively small, and relatively
    /// consistent between frames, the optional shrinking behavior is excluded.
    /// </summary>
    class JobDeque
    {
    public:

        static constexpr size_t DefaultCapacity = 1024;

        JobDeque();
        ~JobDeque();

        /// <summary>
        /// (Owner) Adds a job to the bottom.
        /// </summary>
        /// <param name="job"></param>
        void push(JobHandle job) noexcept;

        /// <summary>
        /// (Owner) Removes a job from the bottom.
        /// </summary>
        /// <returns>Returns null if no jobs are available.</returns>
        std::optional<JobHandle> pop() noexcept;

        /// <summary>
        /// (Thief) Removes a job from the top.
        /// </summary>
        /// <returns>Returns null if no jobs are available.</returns>
        std::optional<JobHandle> steal() noexcept;

        /// <summary>
        /// Cleans (deletes) all outgrown buffers. Should only be called by the JobScheduler.
        /// </summary>
        void clean() noexcept;

        /// <summary>
        /// Returns number of jobs currently in the deque.
        /// For heuristics only. Potentially racy.
        /// </summary>
        /// <returns></returns>
        uint32_t size() const noexcept;

        /// <summary>
        /// Returns how many jobs could be stored in the deque.
        /// </summary>
        /// <returns></returns>
        uint32_t capacity() const noexcept;

    protected:

    private:

        struct RingBuffer;

        // Note that the indices are stored as signed integers. This is intentional.
        // There are cases where bottom could be temporarily less than top - typically during
        // an owner-pop vs thief-steal race to claim the last job. If unsigned integers
        // were used, then the bottom would wrap around to the max value, failing the
        // internal (top < bottom) checks.

        /// <summary>
        /// The next available write index.
        /// Incremented on every push/pop operation.
        /// </summary>
        alignas(Constants::cache_line_size) std::atomic<int64_t> m_bottom;
        // ^ Store the bottom/top indices on different cache lines to avoid false sharing.

        /// <summary>
        /// The next read index.
        /// Incremented on every steal operation.
        /// </summary>
        alignas(Constants::cache_line_size) std::atomic<int64_t> m_top;
        // ^ Store the bottom/top indices on different cache lines to avoid false sharing.

        /// <summary>
        /// The current ring buffer instance servicing the queue.
        /// </summary>
        std::atomic<RingBuffer*> m_pBuffer;

        /// <summary>
        /// Previous ring buffer instances that the queue has grown out of.
        /// </summary>
        std::vector<RingBuffer*> m_deadBuffers;
    };
}

#endif