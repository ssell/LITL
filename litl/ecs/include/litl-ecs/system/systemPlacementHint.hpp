#ifndef LITL_ECS_SYSTEM_PLACEMENT_HINT_H__
#define LITL_ECS_SYSTEM_PLACEMENT_HINT_H__

#include <cstdint>

namespace LITL::ECS
{
    enum SystemPlacementHint : uint32_t
    {
        /// <summary>
        /// Prefer if the system is run first, or as near to first as possible.
        /// </summary>
        First = 0,

        /// <summary>
        /// No preference on order position, aside from its dependencies.
        /// </summary>
        None = 1,

        /// <summary>
        /// Prefer if the system is run last, or as near to last as possible.
        /// </summary>
        Last = 2
    };
}

#endif