#ifndef LITL_ECS_ENTITY_COMMAND_PROCESSOR_H__
#define LITL_ECS_ENTITY_COMMAND_PROCESSOR_H__

#include <vector>

#include "litl-ecs/entity/entityCommands.hpp"
#include "litl-ecs/world.hpp"

namespace LITL::ECS
{
    /// <summary>
    /// Combines one or more EntityCommand buffers, unifies the commands, and then runs them.
    /// </summary>
    class EntityCommandProcessor
    {
    public:

        void process(World* world, std::vector<EntityCommands>& commandBuffers) noexcept;

    protected:

    private:

        std::vector<EntityCommand> m_combinedCommands;
    };
}

#endif