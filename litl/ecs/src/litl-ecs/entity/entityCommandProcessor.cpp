#include <algorithm>
#include <unordered_set>

#include "litl-ecs/entity/entityCommandProcessor.hpp"
#include "litl-ecs/archetype/archetypeRegistry.hpp"


namespace litl
{
    void EntityCommandProcessor::process(World* world, std::vector<EntityCommands*>& incomingCommands, std::vector<EntityChange>& entityChanges) noexcept
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
        ArchetypeId prevArchetype = ecs::Constants::null_archetype_id;
        ArchetypeId currArchetype = ecs::Constants::null_archetype_id;
        bool entityRemoved = false;
        bool archetypeChanged = false;

        for (auto& command : m_combinedCommands)
        {
            if (currEntity != command.entity)
            {
                // Apply the awaiting queued up add/remove component commands
                if (!entityRemoved && archetypeChanged)
                {
                    currArchetype = world->mutateImmediate(currEntity, addedComponents, removedComponents);
                    entityChanges.emplace_back(EntityChangeType::ChangeArchetype, currEntity, prevArchetype, currArchetype);
                }

                // Reset loop state
                currEntity = command.entity;
                prevArchetype = world->getEntityRecord(currEntity).archetype->id();
                currArchetype = prevArchetype;
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

            case EntityCommandType::CreateEntity:
                // Just output that the entity was created
                entityChanges.emplace_back(EntityChangeType::CreateEntity, currEntity, prevArchetype, currArchetype);
                break;

            case EntityCommandType::DestroyEntity:
                entityRemoved = true;
                world->destroyImmediate(currEntity);
                entityChanges.emplace_back(EntityChangeType::DestroyEntity, currEntity, prevArchetype, ecs::Constants::empty_archetype_id);
                break;

            case EntityCommandType::AddComponent:
                archetypeChanged = true;
                addedComponents.emplace_back(command.componentInfo.component, command.componentInfo.data);
                // entityChanges updated in mutate once all component commands have been compiled
                break;

            case EntityCommandType::RemoveComponent:
                archetypeChanged = true;
                removedComponents.emplace_back(command.componentInfo.component);
                // entityChanges updated in mutate once all component commands have been compiled
                break;

            case EntityCommandType::SetParent:
                // This command processor does nothing else for SetParent other than emit that a SetParent was requested.
                // Also for SetParent the archetypes dont matter at the time of processing this command, so just provide whatever is currently set.
                entityChanges.emplace_back(EntityChangeType::SetParent, currEntity, prevArchetype, currArchetype, command.setParentInfo.parent);
                break;

            default:
                break;
            }
        }

        // Once all commands have been processed, it is now safe to reset the internal queues and memory pools.
        for (auto* commandBuffer : incomingCommands)
        {
            commandBuffer->reset();
        }

        m_combinedCommands.clear();
    }
}