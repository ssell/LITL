#ifndef LITL_CORE_TASK_THREAD_POOL_H__
#define LITL_CORE_TASK_THREAD_POOL_H__

#include <condition_variable>
#include <coroutine>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

#include "litl-core/moveOnlyFunc.hpp"

namespace litl
{
    /// <summary>
    /// A pool of worker threads that process tasks.
    /// Primarily used in conjunction with the ResumeTaskOnThreadPool utility.
    /// </summary>
    class TaskThreadPool final
    {
    public:

        explicit TaskThreadPool(uint32_t threadCount);
        ~TaskThreadPool() = default;

        /// <summary>
        /// Adds the coroutine to the queue of work to be run.
        /// </summary>
        /// <param name="handle"></param>
        void schedule(std::coroutine_handle<> handle) noexcept;

        /// <summary>
        /// Adds a custom function to the queue of work to be run.
        /// </summary>
        /// <param name="func"></param>
        void post(MoveOnlyFunc<void()> func) noexcept;

    private:

        /// <summary>
        /// Each worker thread waits for a work item to become available and then processes it.
        /// </summary>
        /// <param name="stop"></param>
        void workerLoop(std::stop_token stop) noexcept;

        std::vector<std::jthread> m_workers;
        std::mutex m_mutex;
        std::condition_variable_any m_conditionVariable;
        std::deque<MoveOnlyFunc<void()>> m_queue;
    };
}

#endif