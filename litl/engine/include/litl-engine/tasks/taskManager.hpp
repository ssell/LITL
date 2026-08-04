#ifndef LITL_ENGINE_TASKS_TASK_MANAGER_H__
#define LITL_ENGINE_TASKS_TASK_MANAGER_H__

#include <coroutine>
#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>

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

        template<typename T>
        void schedule(Task<T>&& task) noexcept
        {
            std::scoped_lock lock(m_mutex);

            auto handle = std::exchange(task.handle, {});

            m_ownedTasks.emplace_back(
                handle,                                                             // the Task coroutine handle
                [](void* addr) noexcept -> bool                                     // type-erased retrieval of Task::promise_type::finished
                {
                    using PromiseType = typename Task<T>::promise_type;
                    return std::coroutine_handle<PromiseType>::from_address(addr).promise().finished.load(std::memory_order_acquire);
                });

            TaskThreadQueue::GetMainThreadQueue().schedule(handle);
        }

    private:

        struct OwnedTask;

        std::vector<OwnedTask> m_ownedTasks;
        std::vector<size_t> m_reapedTasks;
        std::unique_ptr<TaskThreadPool> m_pTaskThreadPool;
        std::mutex m_mutex;

        /// <summary>
        /// Stores the transferred coroutine handle and the resolved isFinished function.
        /// This is the mechanism by which the TaskManager takes ownership of the Task coroutine.
        /// </summary>
        struct OwnedTask
        {
            using IsFinishedFunc = bool (*)(void*) noexcept;

            OwnedTask(std::coroutine_handle<> handle, IsFinishedFunc isFinished) noexcept
                : m_handle(handle), m_isFinished(isFinished)
            {

            }

            OwnedTask(OwnedTask&& other) noexcept
                : m_handle(std::exchange(other.m_handle, {})), m_isFinished(std::exchange(other.m_isFinished, nullptr))
            {

            }

            OwnedTask& operator=(OwnedTask&& other) noexcept
            {
                if (this != &other)
                {
                    reset();
                    m_handle = std::exchange(other.m_handle, {});
                    m_isFinished = std::exchange(other.m_isFinished, nullptr);
                }

                return *this;
            }

            OwnedTask(OwnedTask const&) = delete;
            OwnedTask& operator=(OwnedTask const&) = delete;

            ~OwnedTask()
            {
                reset();
            }

            [[nodiscard]] bool isFinished() const noexcept
            {
                return m_handle && m_isFinished(m_handle.address());
            }

            [[nodiscard]] std::coroutine_handle<> handle() const noexcept
            {
                return m_handle;
            }

        private:

            void reset() noexcept
            {
                if (m_handle)
                {
                    m_handle.destroy();
                    m_handle = nullptr;
                }
            }

            std::coroutine_handle<> m_handle;
            IsFinishedFunc m_isFinished{ nullptr };
        };

        static_assert(std::is_nothrow_move_constructible_v<OwnedTask>);         // vector growth needs to move and not copy
    };
}

#endif