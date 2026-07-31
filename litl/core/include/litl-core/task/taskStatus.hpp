#ifndef LITL_CORE_TASK_STATUS_H__
#define LITL_CORE_TASK_STATUS_H__

#include <optional>

namespace litl
{
    enum class TaskStatusType : uint32_t
    {
        None = 0u,
        Complete = 1u,
        Error = 2u
    };

    template<typename T>
    struct TaskStatus
    {
        TaskStatusType status{ TaskStatusType::None };
        std::optional<T> result{ std::nullopt };
    };
}

#endif