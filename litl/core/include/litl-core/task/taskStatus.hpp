#ifndef LITL_CORE_TASK_STATUS_H__
#define LITL_CORE_TASK_STATUS_H__

#include <optional>

namespace litl
{
    enum class TaskStatusType : uint32_t
    {
        None        = 0u,
        Running     = 1u,
        Complete    = 2u,
        Error       = 3u
    };

    enum class TaskExecutionState : uint32_t
    {
        None        = 0u,
        Yielded     = 1u,
        Awaiting    = 2u,
        Returned    = 3u
    };

    template<typename T>
    struct TaskStatus
    {
        TaskStatusType status{ TaskStatusType::None };
        TaskExecutionState state{ TaskExecutionState::None };
        std::optional<T> value{ std::nullopt };
    };
}

#endif