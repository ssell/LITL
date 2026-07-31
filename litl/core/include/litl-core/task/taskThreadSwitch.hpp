#ifndef LITL_CORE_TASK_UTILS_H__
#define LITL_CORE_TASK_UTILS_H__

#include "litl-core/task/task.hpp"
#include "litl-core/task/taskThreadPool.hpp"
#include "litl-core/task/taskThreadQueue.hpp"

namespace litl
{
    /// <summary>
    /// Utility which can be co_await on to resume execution on a worker thread.
    /// </summary>
    struct ResumeTaskOnWorkerThread final
    {
        TaskThreadPool& pool;

        ResumeTaskOnWorkerThread(TaskThreadPool& pool) : pool{ pool } {}
        ~ResumeTaskOnWorkerThread() = default;

        ResumeTaskOnWorkerThread(ResumeTaskOnWorkerThread const&) = delete;
        ResumeTaskOnWorkerThread& operator=(ResumeTaskOnWorkerThread const&) = delete;

        /// <summary>
        /// Returns false so that the coroutine suspends and we immediately invoke await_suspend.
        /// By hardcoding false, we never enter await_resume.
        /// </summary>
        /// <returns></returns>
        bool await_ready() const noexcept
        {
            return false;
        }

        /// <summary>
        /// Invoked on co_yield to schedule the handle on a worker thread and resume execution from there.
        /// </summary>
        /// <param name="handle"></param>
        void await_suspend(std::coroutine_handle<> handle) const
        {
            pool.schedule(handle);
        }

        /// <summary>
        /// Unused but required.
        /// </summary>
        void await_resume() const noexcept
        {

        }
    };

    /// <summary>
    /// Utility which can be co_await on to resume execution on the main thread.
    /// </summary>
    struct ResumeTaskOnMainThread final
    {
        /// <summary>
        /// Returns true if we are already on main thread and goes to await_resume.
        /// Returns false if we are no on the main thread and goes to await_suspend.
        /// </summary>
        /// <returns></returns>
        bool await_ready() const noexcept
        {
            return TaskThreadQueue::GetMainThreadQueue().isCurrentThread();
        }

        /// <summary>
        /// Moves execution to the main thread by running the coroutine on it.
        /// </summary>
        /// <param name="handle"></param>
        void await_suspend(std::coroutine_handle<> handle) const noexcept
        {
            TaskThreadQueue::GetMainThreadQueue().schedule(handle);
        }

        /// <summary>
        /// Performs no action as we only enter here when already on the main thread.
        /// </summary>
        void await_resume() const noexcept
        {

        }
    };
}

#endif