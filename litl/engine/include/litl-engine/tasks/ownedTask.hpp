#ifndef LITL_ENGINE_TASKS_OWNED_TASK_H__
#define LITL_ENGINE_TASKS_OWNED_TASK_H__

#include <coroutine>
#include <type_traits>
#include <utility>

#include "litl-core/constants.hpp"

namespace litl
{
    using TaskId = uint32_t;

    /// <summary>
    /// Stores the transferred coroutine handle and the resolved isFinished function.
    /// This is the mechanism by which the TaskManager takes ownership of the Task coroutine.
    /// </summary>
    struct OwnedTask
    {
        using IsFinishedFunc = bool (*)(void*) noexcept;

        OwnedTask(TaskId id, std::coroutine_handle<> handle, IsFinishedFunc isFinished, bool destroyOnComplete) noexcept;
        OwnedTask(OwnedTask&& other) noexcept;
        OwnedTask& operator=(OwnedTask&& other) noexcept;
        ~OwnedTask();

        OwnedTask(OwnedTask const&) = delete;
        OwnedTask& operator=(OwnedTask const&) = delete;

        [[nodiscard]] TaskId id() const noexcept;
        [[nodiscard]] std::coroutine_handle<> handle() const noexcept;
        [[nodiscard]] bool isFinished() const noexcept;
        [[nodiscard]] bool shouldDestroyOnComplete() const noexcept;

    private:

        void reset() noexcept;

        std::coroutine_handle<> m_handle;
        IsFinishedFunc m_isFinished{ nullptr };
        TaskId m_id{ Constants::uint32_null_index };
        bool m_destroyOnComplete{ false };
    };

    static_assert(std::is_nothrow_move_constructible_v<OwnedTask>);         // vector growth needs to move and not copy
}

#endif