#ifndef LITL_ECS_SYSTEM_EXECUTION_H__
#define LITL_ECS_SYSTEM_EXECUTION_H__

#include <cstdint>

namespace litl
{
    enum class SystemExecution : uint32_t
    {
        /// <summary>
        /// The default execution style for all systems.
        /// 
        /// Each archetype chunk is run on its own job, but all jobs are run in parallel alongside
        /// other system chunks and all other systems within the same execution layer.
        /// </summary>
        Parallel = 0u,

        /// <summary>
        /// Archetype chunks are run sequentially with no other chunks (from the same or other systems)
        /// running at the same time. This should be reserved for small systems who require
        /// access to external services that are not thread-safe.
        /// </summary>
        Exclusive
    };
}

#endif