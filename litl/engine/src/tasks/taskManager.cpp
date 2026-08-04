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
    TaskManager::TaskManager()
        : m_pTaskThreadPool(nullptr)
    {

    }

    TaskManager::~TaskManager()
    {
        // ... needed as this is an injected service and will reside in a shared_ptr ...
    }

    void TaskManager::setup(Authority<Engine> auth, ServiceProvider& services) noexcept
    {
        LITL_FATAL_ASSERT_MSG(ThreadInfo::isMainThread(), "TaskManager::setup run from a thread that is not the main thread.");
        TaskThreadQueue::RegisterMainThreadQueue();

        auto config = services.get<Configuration>();
        LITL_FATAL_ASSERT_MSG((config != nullptr), "Failed to inject Configuration into TaskManager");
        m_pTaskThreadPool = std::make_unique<TaskThreadPool>(config->engineSettings.taskThreadCount);
    }

    void TaskManager::destroy(Authority<Engine> auth) noexcept
    {

    }

    void TaskManager::update() noexcept
    {
        TaskThreadQueue::GetMainThreadQueue().drain();
    }
}