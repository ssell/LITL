#include "litl-ecs/entity/entityCommands.hpp"

namespace LITL::ECS
{
    EntityCommands::EntityCommands()
        : m_nextId(0)
    {

    }

    EntityCommands::~EntityCommands()
    {

    }

    EntityCommandQueue& EntityCommands::queue() noexcept
    {
        return m_commands;
    }

    DeferredEntity EntityCommands::createEntity() noexcept
    {
        DeferredEntity entity = { static_cast<uint32_t>(m_nextId++) };

        m_commands.push({
            .type = EntityCommandType::CreateEntity,
            .deferredEntity = entity
        });

        return entity;
    }

    void EntityCommands::destroyEntity(Entity entity) noexcept
    {
        m_commands.push({
            .type = EntityCommandType::DestroyEntity,
            .entity = entity
        });
    }

    void EntityCommands::destroyEntity(DeferredEntity entity) noexcept
    {
        m_commands.push({
            .type = EntityCommandType::DestroyEntity,
            .deferredEntity = entity
        });
    }

    void EntityCommands::addComponent(Entity entity, ComponentTypeId component, void* data) noexcept
    {
        m_commands.push({
            .type = EntityCommandType::AddComponent,
            .entity = entity,
            .component = component
        }, data);
    }

    void EntityCommands::addComponent(DeferredEntity entity, ComponentTypeId component, void* data) noexcept
    {
        m_commands.push({
            .type = EntityCommandType::AddComponent,
            .deferredEntity = entity,
            .component = component
        }, data);
    }

    void EntityCommands::removeComponent(Entity entity, ComponentTypeId component) noexcept
    {
        m_commands.push({
            .type = EntityCommandType::RemoveComponent,
            .entity = entity,
            .component = component
        });
    }

    void EntityCommands::removeComponent(DeferredEntity entity, ComponentTypeId component) noexcept
    {
        m_commands.push({
            .type = EntityCommandType::RemoveComponent,
            .deferredEntity = entity,
            .component = component
        });
    }
}