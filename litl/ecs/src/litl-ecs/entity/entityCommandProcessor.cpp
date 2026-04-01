#include <algorithm>
#include <unordered_set>

#include "litl-ecs/entity/entityCommandProcessor.hpp"
#include "litl-ecs/entity/entityRegistry.hpp"

namespace LITL::ECS
{
    void EntityCommandProcessor::process(World* world, std::vector<EntityCommands>& commandBuffers) noexcept
    {
        size_t totalCommandCount = 0;
        size_t offset = 0;
        size_t nextOffset = 0;

        for (auto& commandBuffer : commandBuffers)
        {
            totalCommandCount += commandBuffer.actionableCommandCount();
        }

        if (totalCommandCount > m_combinedCommands.size())
        {
            m_combinedCommands.resize(totalCommandCount);
        }

        // Combine all command buffers
        for (auto& commandBuffer : commandBuffers)
        {
            nextOffset = offset + commandBuffer.actionableCommandCount();
            commandBuffer.extractCommands(world, m_combinedCommands, offset);
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
                        return a.component < b.component;
                    }
                }
            });

        std::vector<ComponentData> addedComponents;
        addedComponents.reserve(Constants::max_components * 2);

        std::vector<ComponentTypeId> removedComponents;
        removedComponents.reserve(Constants::max_components * 2);

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

            if (command.type == EntityCommandType::DestroyEntity)
            {
                entityRemoved = true;
                world->destroyImmediate(currEntity);
            }
            else if (command.type == EntityCommandType::AddComponent)
            {
                archetypeChanged = true;
                addedComponents.emplace_back(command.component, command.data);
            }
            else if (command.type == EntityCommandType::RemoveComponent)
            {
                archetypeChanged = true;
                removedComponents.emplace_back(command.component);
            }
        }

        // Once all commands have been processed, it is now safe to reset the internal queues and memory pools.
        for (auto& commandBuffer : commandBuffers)
        {
            commandBuffer.reset();
        }
    }
}