#ifndef LITL_ENGINE_ECS_SCHEDULE_NODE_H__
#define LITL_ENGINE_ECS_SCHEDULE_NODE_H__

#include <atomic>
#include <cstdint>
#include <unordered_set>
#include <vector>

#include "litl-ecs/constants.hpp"
#include "litl-ecs/system/systemPlacementHint.hpp"
#include "litl-ecs/system/systemTraits.hpp"

namespace litl
{
    /// <summary>
    /// A single scheduled system.
    /// </summary>
    struct SystemNode
    {
        explicit SystemNode(SystemTypeId systemIndex, std::vector<SystemComponentInfo> const& components)
            : systemId(systemIndex), componentInfo(components)
        {

        }

        auto operator<=>(SystemNode const& other) const
        {
            return static_cast<uint32_t>(placement) <=> static_cast<uint32_t>(other.placement);
        }

        bool operator==(SystemNode const& other) const = default;

        /// <summary>
        /// The system instance referenced by this node.
        /// </summary>
        SystemTypeId systemId{ 0 };

        /// <summary>
        /// Optional hint as to where this system should be ordered.
        /// </summary>
        SystemPlacementHint placement{ SystemPlacementHint::None };

        /// <summary>
        /// The access levels required by the system for the components it operates on.
        /// </summary>
        std::vector<SystemComponentInfo> componentInfo{};

        /// <summary>
        /// The system nodes (represented by their index in the group) that are dependent on this system.
        /// </summary>
        std::unordered_set<uint32_t> outgoing{};
    };
}

#endif