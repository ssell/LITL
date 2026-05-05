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
        /// and outputs a list of EntityChanges which represent the changes made to each entity. The number of resulting
        /// changes can be less than the number of incoming commands, as some commands (such as Destroy) can cancel out other commands.
        /// </summary>
        /// <param name="world"></param>
        /// <param name="incomingCommands"></param>
        /// <param name="outgoingCommands"></param>
        void process(World* world, std::vector<EntityCommands*>& incomingCommands, std::vector<EntityChange>& entityChanges) noexcept;

    protected:

    private:

        /// <summary>
        /// Keep a local vector so that eventually we stop having to allocate/resize it.
        /// </summary>
        std::vector<EntityCommand> m_combinedCommands;
    };
}

#endif