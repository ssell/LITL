#ifndef LITL_ECS_ENTITY_COMMAND_PROCESSOR_H__
#define LITL_ECS_ENTITY_COMMAND_PROCESSOR_H__

#include <span>
#include <vector>

#include "litl-ecs/entity/entityCommands.hpp"
#include "litl-ecs/world.hpp"

namespace litl
{
    /// <summary>
    /// Combines one or more EntityCommand buffers, unifies the commands, and then runs them.
    /// </summary>
    class EntityCommandProcessor
    {
    public:

        /// <summary>
        /// Processes an incoming list of EntityCommands (each of which may contain zero or more EntityCommand requests)
        /// and outputs a list of EntitySceneCommands that represent structural scene changes. The scene changes are not
        /// relevant to the ECS itself, but may be relevant to any scene implementation that is integrating with the ECS.
        /// </summary>
        /// <param name="world"></param>
        /// <param name="incomingCommands"></param>
        /// <param name="outgoingCommands"></param>
        void process(World* world, std::vector<EntityCommands*>& incomingCommands, std::vector<EntityCommand>& outgoingCommands) noexcept;

    protected:

    private:

        /// <summary>
        /// Keep a local vector so that eventually we stop having to allocate/resize it.
        /// </summary>
        std::vector<EntityCommand> m_combinedCommands;
    };
}

#endif