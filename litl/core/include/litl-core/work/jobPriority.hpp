#ifndef LITL_CORE_WORK_JOB_PRIORITY_H__
#define LITL_CORE_WORK_JOB_PRIORITY_H__

#include <cstdint>

namespace LITL::Core
{
    /// <summary>
    /// Prioritizes a job within the WorkScheduler.
    /// 
    /// Higher priority jobs will be picked up by a Worker before lower priority jobs.
    /// </summary>
    enum class JobPriority : uint32_t
    {
        /// <summary>
        /// For jobs that should be prioritized above all others.
        /// These are critical jobs such as ECS systems, frame-critical work, etc.
        /// </summary>
        High = 0,

        /// <summary>
        /// The default priority level. For general gameplay tasks, etc.
        /// </summary>
        Normal = 1,

        /// <summary>
        /// Background jobs such as asset compression/decompression, streaming, etc.
        /// </summary>
        Low = 2,

        __JobPriorityCount
    };

    static constexpr uint32_t JobPriorityCount = static_cast<uint32_t>(LITL::Core::JobPriority::__JobPriorityCount);
}

#endif