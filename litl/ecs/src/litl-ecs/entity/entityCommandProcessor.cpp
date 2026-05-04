#include <algorithm>
#include <unordered_set>

#include "litl-ecs/entity/entityCommandProcessor.hpp"

namespace litl
{
    void EntityCommandProcessor::process(World* world, std::vector<EntityCommands*>& incomingCommands, std::vector<EntityCommand>& outgoingCommands) noexcept
    {
        for (auto* commandBuffer : incomingCommands)
        {
            assert(commandBuffer != nullptr);
        }

        size_t totalCommandCount = 0;
        size_t offset = 0;
        size_t nextOffset = 0;

        for (auto* commandBuffer : incomingCommands)
        {
            totalCommandCount += commandBuffer->actionableCommandCount();
        }

        if (totalCommandCount > m_combinedCommands.size())
        {
            m_combinedCommands.resize(totalCommandCount);
        }

        // Combine all command buffers
        for (auto* commandBuffer : incomingCommands)
        {
            nextOffset = offset + commandBuffer->actionableCommandCount();
            commandBuffer->extractCommands(world, m_combinedCommands, offset);
            offset = nextOffset;
        }

        // Sort based on: entity -> command type -> component
        std::sort(m_combinedCommands.begin(), m_combinedCommands.end(), [](EntityCommand a, EntityCommand b) -> bool
            {
                if (a.entity != b.entity)
                {
                    return (a.entity.index < b.entity.index);
                }
                else
                {
                    if (a.type != b.type)
                    {
                        return static_cast<uint32_t>(a.type) < static_cast<uint32_t>(b.type);
                    }
                    else
                    {
                        return a.componentInfo.component < b.componentInfo.component;
                    }
                }
            });

        std::vector<ComponentData> addedComponents;
        addedComponents.reserve(ecs::Constants::max_components * 2);

        std::vector<ComponentTypeId> removedComponents;
        removedComponents.reserve(ecs::Constants::max_components * 2);

        Entity currEntity{};
        bool entityRemoved = false;
        bool archetypeChanged = false;

        for (auto& command : m_combinedCommands)
        {
            if (currEntity != command.entity)
            {
                // Apply the awaiting queued up add/remove component commands
                if (!entityRemoved && archetypeChanged)
                {
                    world->mutateImmediate(currEntity, addedComponents, removedComponents);
                }

                // Reset loop state
                currEntity = command.entity;
                entityRemoved = false;
                archetypeChanged = false;

                addedComponents.clear();
                removedComponents.clear();
            }

            // If the entity was removed by an earlier command then skip this one.
            if (entityRemoved)
            {
                continue;
            }

            switch (command.type)
            {
            case EntityCommandType::DestroyEntity:
                entityRemoved = true;
                world->destroyImmediate(currEntity);
                break;

            case EntityCommandType::AddComponent:
                archetypeChanged = true;
                addedComponents.emplace_back(command.componentInfo.component, command.componentInfo.data);
                break;

            case EntityCommandType::RemoveComponent:
                archetypeChanged = true;
                removedComponents.emplace_back(command.componentInfo.component);
                break;
            }

            // Record the command. If there was a previous destroy command then none of the following commands for that entity are recorded.
            outgoingCommands.push_back(command);
        }

        // Once all commands have been processed, it is now safe to reset the internal queues and memory pools.
        for (auto* commandBuffer : incomingCommands)
        {
            commandBuffer->reset();
        }

        m_combinedCommands.clear();
    }
}