#include "litl-core/task/taskThreadQueue.hpp"

namespace litl
{
    void TaskThreadQueue::RegisterMainThreadQueue() noexcept
    {
        GetMainThreadQueue();
    }

    TaskThreadQueue& TaskThreadQueue::GetMainThreadQueue() noexcept
    {
        static TaskThreadQueue MainThreadQueue{};
        return MainThreadQueue;
    }

    bool TaskThreadQueue::isCurrentThread() const noexcept
    {
        return std::this_thread::get_id() == m_ownerThreadId;
    }

    void TaskThreadQueue::schedule(std::coroutine_handle<> handle) noexcept
    {
        std::scoped_lock lock{ m_mutex };
        m_pendingHandles.push_back(handle);
    }

    void TaskThreadQueue::drain() noexcept
    {
        {
            std::scoped_lock lock{ m_mutex };
            std::swap(m_pendingHandles, m_drainingHandles);
        }

        for (auto handle : m_drainingHandles)
        {
            handle.resume();
        }

        m_drainingHandles.clear();
    }
}