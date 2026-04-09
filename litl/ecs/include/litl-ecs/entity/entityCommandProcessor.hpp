#ifndef LITL_ECS_ENTITY_COMMAND_PROCESSOR_H__
#define LITL_ECS_ENTITY_COMMAND_PROCESSOR_H__

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

        void process(World* world, std::vector<EntityCommands*>& commandBuffers) noexcept;

    protected:

    private:

        /// <summary>
        /// Keep a local vector so that eventually we stop having to allocate/resize it.
        /// </summary>
        std::vector<EntityCommand> m_combinedCommands;
    };
}

#endif