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

        for (auto& commandBuffer : commandBuffers)
        {
            nextOffset = offset + commandBuffer.actionableCommandCount();
            commandBuffer.extractCommands(world, m_combinedCommands, offset);
            offset = nextOffset;
        }

        std::sort(m_combinedCommands.begin(), m_combinedCommands.end(), [](EntityCommand a, EntityCommand b) -> bool
            {
                // Sort by: entity -> command type -> component
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

        std::vector<ComponentTypeId> addedComponents;
        addedComponents.reserve(Constants::max_components);

        std::vector<ComponentTypeId> removedComponents;
        removedComponents.reserve(Constants::max_components);

        EntityRecord currEntity{};
        bool entityRemoved = false;
        bool archetypeChanged = false;

        // ... todo need to update world with a way to add a component and set the data from an arbitrary address in one go
        // ... and to do that with multiple components at once ...

        for (auto& command : m_combinedCommands)
        {
            if (currEntity.entity != command.entity)
            {
                currEntity = EntityRegistry::getRecord(command.entity);

                if (!entityRemoved && archetypeChanged)
                {

                }

                entityRemoved = false;
                archetypeChanged = false;

                addedComponents.clear();
                removedComponents.clear();
            }

            if (entityRemoved)
            {
                continue;
            }

            if (command.type == EntityCommandType::DestroyEntity)
            {
                entityRemoved = true;
                world->destroyImmediate(currEntity.entity);
            }
            else if (command.type == EntityCommandType::AddComponent)
            {
                archetypeChanged = true;
            }
            else if (command.type == EntityCommandType::RemoveComponent)
            {
                archetypeChanged = true;
            }
        }
    }
}