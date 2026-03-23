#ifndef LITL_ENGINE_ECS_SCHEDULE_NODE_H__
#define LITL_ENGINE_ECS_SCHEDULE_NODE_H__

#include <atomic>
#include <cstdint>
#include <vector>

#include "litl-ecs/constants.hpp"
#include "litl-ecs/system/systemTraits.hpp"

namespace LITL::ECS
{
    enum SystemNodePlacementHint
    {
        /// <summary>
        /// No preference on order position, aside from its dependencies.
        /// </summary>
        None = 0,

        /// <summary>
        /// Prefer if the system is run first, or as near to first as possible.
        /// </summary>
        First = 1,

        /// <summary>
        /// Prefer if the system is run last, or as near to last as possible.
        /// </summary>
        Last = 2
    };

    /// <summary>
    /// A single scheduled system.
    /// </summary>
    struct SystemNode
    {
        explicit SystemNode(SystemTypeId systemIndex, std::vector<SystemComponentInfo> const& components)
            : systemId(systemIndex), componentInfo(components)
        {
            incoming.reserve(8);
            outgoing.reserve(8);
        }

        /// <summary>
        /// The system instance referenced by this node.
        /// </summary>
        SystemTypeId systemId{ 0 };

        /// <summary>
        /// Optional hint as to where this system should be ordered.
        /// </summary>
        SystemNodePlacementHint placement{ SystemNodePlacementHint::None };

        /// <summary>
        /// The access levels required by the system for the components it operates on.
        /// </summary>
        std::vector<SystemComponentInfo> componentInfo{};

        /// <summary>
        /// The system nodes (represented by their index in the group) that this system is dependent on.
        /// </summary>
        std::vector<uint32_t> incoming{};

        /// <summary>
        /// The system nodes (represented by their index in the group) that are dependent on this system.
        /// </summary>
        std::vector<uint32_t> outgoing{};

        /// <summary>
        /// The number of incoming systems that still need to finish before this system can be executed.
        /// The scheduler does not consider "what system should be run next?" but rather "what systems have zero remaining incoming dependencies?"
        /// </summary>
        //std::atomic<uint32_t> remainingIncoming;
    };
}

#endif