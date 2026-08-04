#ifndef LITL_ENGINE_TASKS_TASK_MANAGER_H__
#define LITL_ENGINE_TASKS_TASK_MANAGER_H__

#include <coroutine>
#include <memory>
#include <mutex>
#include <utility>

#include "litl-core/authority.hpp"
#include "litl-core/impl.hpp"
#include "litl-core/task/task.hpp"
#include "litl-core/task/taskThreadQueue.hpp"

namespace litl
{
    class Engine;
    class ServiceProvider;
    class TaskThreadPool;

    /// <summary>
    /// Manages the engine-specific task thread pool.
    /// </summary>
    class TaskManager final
    {
        struct OwnedTask final
        {
            ~OwnedTask() { if (handle) { handle.destroy(); } }
            std::coroutine_handle<> handle;
        };

    public:

        TaskManager();
        ~TaskManager();

        TaskManager(TaskManager const&) = delete;
        TaskManager& operator=(TaskManager const&) = delete;

        void setup(Authority<Engine> auth, ServiceProvider& services) noexcept;
        void destroy(Authority<Engine> auth) noexcept;
        void update() noexcept;

        template<typename T>
        void schedule(Task<T>&& task) noexcept
        {
            std::scoped_lock lock(m_mutex);
            m_ownedTasks.emplace_back(std::exchange(task.handle, {}));
            TaskThreadQueue::GetMainThreadQueue().schedule(m_ownedTasks.back().handle);
        }

    private:

        std::vector<OwnedTask> m_ownedTasks;
        std::unique_ptr<TaskThreadPool> m_pTaskThreadPool;
        std::mutex m_mutex;
    };
}

#endif