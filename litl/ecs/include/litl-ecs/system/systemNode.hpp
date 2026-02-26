#ifndef LITL_ENGINE_ECS_SCHEDULE_NODE_H__
#define LITL_ENGINE_ECS_SCHEDULE_NODE_H__

#include <atomic>
#include <cstdint>
#include <vector>

#include "litl-ecs/constants.hpp"
#include "litl-ecs/system/systemComponentAccess.hpp"

namespace LITL::ECS
{
    /// <summary>
    /// A single scheduled system.
    /// </summary>
    struct SystemNode
    {
        SystemNode(SystemTypeId systemIndex)
            : systemId(systemIndex)
        {
            componentAccess.reserve(8);
            incoming.reserve(8);
            outgoing.reserve(8);
        }

        /// <summary>
        /// The system instance referenced by this node.
        /// </summary>
        SystemTypeId systemId;

        /// <summary>
        /// The access levels required by the system for the components it operates on.
        /// </summary>
        std::vector<SystemComponentAccess> componentAccess;

        /// <summary>
        /// The system nodes (represented by their index in the group) that this system is dependent on.
        /// </summary>
        std::vector<uint32_t> incoming;

        /// <summary>
        /// The system nodes (represented by their index in the group) that are dependent on this system.
        /// </summary>
        std::vector<uint32_t> outgoing;

        /// <summary>
        /// The number of incoming systems that still need to finish before this system can be executed.
        /// The scheduler does not consider "what system should be run next?" but rather "what systems have zero remaining incoming dependencies?"
        /// </summary>
        //std::atomic<uint32_t> remainingIncoming;
    };
}

#endif