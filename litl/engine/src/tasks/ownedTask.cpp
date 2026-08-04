#include "litl-engine/tasks/ownedTask.hpp"

namespace litl
{
    OwnedTask::OwnedTask(TaskId id, std::coroutine_handle<> handle, IsFinishedFunc isFinished, bool destroyOnComplete) noexcept :
        m_id(id),
        m_handle(handle),
        m_isFinished(isFinished),
        m_destroyOnComplete(destroyOnComplete)
    {

    }

    OwnedTask::OwnedTask(OwnedTask&& other) noexcept :
        m_id(std::exchange(other.m_id, Constants::uint32_null_index)),
        m_handle(std::exchange(other.m_handle, {})),
        m_isFinished(std::exchange(other.m_isFinished, nullptr)),
        m_destroyOnComplete(std::exchange(other.m_destroyOnComplete, false))
    {

    }

    OwnedTask& OwnedTask::operator=(OwnedTask&& other) noexcept
    {
        if (this != &other)
        {
            reset();

            m_id = std::exchange(other.m_id, Constants::uint32_null_index);
            m_handle = std::exchange(other.m_handle, {});
            m_isFinished = std::exchange(other.m_isFinished, nullptr);
            m_destroyOnComplete = std::exchange(other.m_destroyOnComplete, false);
        }

        return *this;
    }

    OwnedTask::~OwnedTask()
    {
        reset();
    }

    void OwnedTask::reset() noexcept
    {
        if (m_handle)
        {
            m_id = Constants::uint32_null_index;
            m_handle.destroy(); m_handle = nullptr;
            m_destroyOnComplete = false;
        }
    }

    TaskId OwnedTask::id() const noexcept
    {
        return m_id;
    }

    std::coroutine_handle<> OwnedTask::handle() const noexcept
    {
        return m_handle;
    }

    bool OwnedTask::isFinished() const noexcept
    {
        return m_handle && m_isFinished(m_handle.address());
    }

    bool OwnedTask::shouldDestroyOnComplete() const noexcept
    {
        return m_destroyOnComplete;
    }
}