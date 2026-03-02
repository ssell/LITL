#ifndef LITL_CORE_WORK_QUEUE_H__
#define LITL_CORE_WORK_QUEUE_H__

#include <atomic>
#include <cstdint>

#include "litl-core/alignment.hpp"
#include "litl-core/work/job.hpp"

namespace LITL::Core
{
    /// <summary>
    /// Implementation of a Chase-Lev work-stealing deque.
    /// </summary>
    class WorkQueue
    {
    public:

        WorkQueue();
        ~WorkQueue();

        /// <summary>
        /// Enqueues a job into the queue.
        /// Should be invoked only on the owner thread.
        /// </summary>
        /// <param name="job"></param>
        void push(Job* job) noexcept;

        /// <summary>
        /// Dequeues a job from the queue.
        /// Should be invoked only on the owner thread.
        /// </summary>
        /// <returns></returns>
        Job* pop() noexcept;

        /// <summary>
        /// Steals a job from the queue. 
        /// Can be called from any thread.
        /// </summary>
        /// <returns></returns>
        Job* steal() noexcept;

    protected:

    private:

        struct RingBuffer;

        // Store that write/read indices on different cache lines to avoid false sharing.
        alignas(CacheLineSize) std::atomic<uint32_t> m_bottom;
        alignas(CacheLineSize) std::atomic<uint32_t> m_top;

        std::atomic<RingBuffer*> m_pBuffer;

    };
}

#endif