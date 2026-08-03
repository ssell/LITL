#ifndef LITL_ENGINE_TASKS_TASK_MANAGER_H__
#define LITL_ENGINE_TASKS_TASK_MANAGER_H__

#include "litl-core/authority.hpp"
#include "litl-core/impl.hpp"

namespace litl
{
    class Engine;
    class ServiceProvider;

    /// <summary>
    /// Manages the engine-specific task thread pool.
    /// </summary>
    class TaskManager final
    {
    public:

        TaskManager();
        ~TaskManager();

        TaskManager(TaskManager const&) = delete;
        TaskManager& operator=(TaskManager const&) = delete;

        void setup(Authority<Engine> auth, ServiceProvider& services) noexcept;
        void destroy(Authority<Engine> auth);
        void update() noexcept;

    private:

        struct Impl;
        ImplPtr<Impl, 256u> m_impl;
    };
}

#endif