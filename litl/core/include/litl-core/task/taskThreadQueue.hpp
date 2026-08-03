#ifndef LITL_CORE_TASK_THREAD_QUEUE_H__
#define LITL_CORE_TASK_THREAD_QUEUE_H__

#include <coroutine>
#include <mutex>
#include <thread>
#include <vector>

namespace litl
{
    /// <summary>
    /// A simple push/drain queue for processing tasks.
    /// When a task is scheduled it will be resumed on the next drain.
    /// </summary>
    class TaskThreadQueue final
    {
    public:

        TaskThreadQueue() = default;
        ~TaskThreadQueue() = default;

        TaskThreadQueue(TaskThreadQueue const&) = delete;
        TaskThreadQueue& operator=(TaskThreadQueue const&) = delete;

        /// <summary>
        /// Invoked once by whatever is the main thread.
        /// </summary>
        static void RegisterMainThreadQueue() noexcept;

        /// <summary>
        /// Retrieves the queue registered to the main thread.
        /// </summary>
        /// <returns></returns>
        static TaskThreadQueue& GetMainThreadQueue() noexcept;

        /// <summary>
        /// Is the current thread the one in which this was created?
        /// </summary>
        /// <returns></returns>
        bool isCurrentThread() const noexcept;

        /// <summary>
        /// Places the coroutine handle into the pending handle vector.
        /// It will be resumed on the next call to drain.
        /// </summary>
        /// <param name="handle"></param>
        void schedule(std::coroutine_handle<> handle) noexcept;

        /// <summary>
        /// Swaps the pending and draining handles and resumes all draining handles (previously pending).
        /// </summary>
        void drain() noexcept;

    private:

        mutable std::mutex m_mutex;
        std::vector<std::coroutine_handle<>> m_pendingHandles;
        std::vector<std::coroutine_handle<>> m_drainingHandles;
        std::thread::id m_ownerThreadId = std::this_thread::get_id();
    };
}

#endif