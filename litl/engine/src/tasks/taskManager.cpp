#include <memory>

#include "litl-core/assert.hpp"
#include "litl-core/thread.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-core/task/taskThreadPool.hpp"
#include "litl-core/task/taskThreadQueue.hpp"
#include "litl-engine/tasks/taskManager.hpp"
#include "litl-engine/engine.hpp"

namespace litl
{
    struct TaskManager::Impl
    {
        /// <summary>
        /// Queue of tasks that are waiting to return back to the main thread.
        /// </summary>
        TaskThreadQueue taskThreadQueue{};

        /// <summary>
        /// Our task-specific pool of worker threads.
        /// </summary>
        std::unique_ptr<TaskThreadPool> taskThreadPool;
    };

    TaskManager::TaskManager()
    {

    }

    TaskManager::~TaskManager()
    {
        // ... needed as this is an injected service and will reside in a shared_ptr ...
    }

    void TaskManager::setup(Authority<Engine> auth, ServiceProvider& services) noexcept
    {
        LITL_FATAL_ASSERT_MSG(ThreadInfo::isMainThread(), "TaskManager::setup run from a thread that is not the main thread.");
        m_impl->taskThreadQueue.RegisterMainThreadQueue();

        auto config = services.get<Configuration>();
        LITL_FATAL_ASSERT_MSG((config != nullptr), "Failed to inject Configuration into TaskManager");
        m_impl->taskThreadPool = std::make_unique<TaskThreadPool>(config->engineSettings.taskThreadCount);
    }

    void TaskManager::destroy(Authority<Engine> auth) noexcept
    {

    }

    void TaskManager::update() noexcept
    {
        m_impl->taskThreadQueue.drain();
    }
}