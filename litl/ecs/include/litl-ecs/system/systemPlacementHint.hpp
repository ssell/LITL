#ifndef LITL_ECS_SYSTEM_PLACEMENT_HINT_H__
#define LITL_ECS_SYSTEM_PLACEMENT_HINT_H__

#include <cstdint>

namespace LITL::ECS
{
    enum class SystemPlacementHint : uint32_t
    {
        /// <summary>
        /// Prefer if the system is run first, or as near to first as possible.
        /// A system marked with a hint of first may not have any direct dependencies.
        /// Doing so will lead to a invalid (and potentially cyclic) system graph.
        /// </summary>
        First = 0,

        /// <summary>
        /// No preference on order position, aside from its dependencies.
        /// </summary>
        None = 1,

        /// <summary>
        /// Prefer if the system is run last, or as near to last as possible.
        /// A system marked with a hint of first may not have any direct dependents.
        /// Doing so will lead to a invalid (and potentially cyclic) system graph.
        /// </summary>
        Last = 2
    };
}

#endif