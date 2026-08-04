#ifndef LITL_ENGINE_TASKS_TASK_MANAGER_H__
#define LITL_ENGINE_TASKS_TASK_MANAGER_H__

#include <memory>
#include <mutex>
#include <vector>

#include "litl-core/authority.hpp"
#include "litl-core/impl.hpp"
#include "litl-core/task/task.hpp"
#include "litl-core/task/taskThreadQueue.hpp"
#include "litl-engine/tasks/ownedTask.hpp"

namespace litl
{
    class Engine;
    class ServiceProvider;
    class TaskThreadPool;

    /// <summary>
    /// Manages the engine-specific task thread pool and queue.
    /// </summary>
    class TaskManager final
    {
    public:

        TaskManager();
        ~TaskManager();

        TaskManager(TaskManager const&) = delete;
        TaskManager& operator=(TaskManager const&) = delete;

        void setup(Authority<Engine> auth, ServiceProvider& services) noexcept;
        void destroy(Authority<Engine> auth) noexcept;
        void update() noexcept;
        void releaseTask(TaskId id) noexcept;

        /// <summary>
        /// Schedules a task to begin running on the next available worker thread.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="task"></param>
        /// <param name="destroyOnComplete">If true, the task will be automatically destroyed when it is finished running. Otherwise it will persist until instructed to remove it.</param>
        /// <returns></returns>
        template<typename T>
        TaskId schedule(Task<T>&& task, bool destroyOnComplete) noexcept
        {
            std::scoped_lock lock(m_ownedTasksMutex);

            auto id = nextId();
            auto handle = std::exchange(task.handle, {});

            // transfer ownership of the task to the manager to ensure its lifetime remains valid
            m_ownedTasks.emplace_back(
                id,
                handle,                                                             // the Task coroutine handle
                [](void* addr) noexcept -> bool                                     // type-erased retrieval of Task::promise_type::finished
                {
                    using PromiseType = typename Task<T>::promise_type;
                    return std::coroutine_handle<PromiseType>::from_address(addr).promise().finished.load(std::memory_order_acquire);
                },
                destroyOnComplete);

            TaskThreadQueue::GetMainThreadQueue().schedule(handle);

            return id;
        }

    private:

        [[nodiscard]] static uint32_t nextId() noexcept;

        std::vector<OwnedTask> m_ownedTasks;
        std::vector<size_t> m_reapedTasks;
        std::unique_ptr<TaskThreadPool> m_pTaskThreadPool;
        std::mutex m_ownedTasksMutex;
    };
}

#endif