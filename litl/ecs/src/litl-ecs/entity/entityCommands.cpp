#include "litl-ecs/entity/entityCommands.hpp"
#include "litl-ecs/world.hpp"

namespace LITL::ECS
{
    EntityCommands::EntityCommands()
        : m_nextId(0)
    {

    }

    EntityCommands::~EntityCommands()
    {

    }

    void EntityCommands::reset() noexcept
    {
        m_nextId = 0;
        m_commands.reset();
    }

    size_t EntityCommands::actionableCommandCount() const noexcept
    {
        return m_commands.actionableCommandCount();
    }

    void EntityCommands::extractCommands(World* world, std::vector<EntityCommand>& commands, size_t offset) noexcept
    {
        assert(world != nullptr);
        assert(commands.size() >= (m_commands.count() + offset));

        materialize(world);

        auto count = m_commands.count();

        for (auto i = offset; i < (offset + count); ++i)
        {
            commands[i] = m_commands.next().value();
        }
    }

    DeferredEntity EntityCommands::createEntity() noexcept
    {
        DeferredEntity entity = { static_cast<uint32_t>(m_nextId++) };

        m_commands.push(DeferredEntityCommand {
            .type = EntityCommandType::CreateEntity,
            .deferredEntity = entity
        });

        return entity;
    }

    void EntityCommands::destroyEntity(Entity entity) noexcept
    {
        m_commands.push(EntityCommand {
            .type = EntityCommandType::DestroyEntity,
            .entity = entity
        });
    }

    void EntityCommands::destroyEntity(DeferredEntity entity) noexcept
    {
        m_commands.push(DeferredEntityCommand {
            .type = EntityCommandType::DestroyEntity,
            .deferredEntity = entity
        });
    }

    void EntityCommands::addComponent(Entity entity, ComponentTypeId component, void* data) noexcept
    {
        m_commands.push(EntityCommand {
            .type = EntityCommandType::AddComponent,
            .entity = entity,
            .component = component
        }, data);
    }

    void EntityCommands::addComponent(DeferredEntity entity, ComponentTypeId component, void* data) noexcept
    {
        m_commands.push(DeferredEntityCommand {
            .type = EntityCommandType::AddComponent,
            .deferredEntity = entity,
            .component = component
        }, data);
    }

    void EntityCommands::removeComponent(Entity entity, ComponentTypeId component) noexcept
    {
        m_commands.push(EntityCommand {
            .type = EntityCommandType::RemoveComponent,
            .entity = entity,
            .component = component
        });
    }

    void EntityCommands::removeComponent(DeferredEntity entity, ComponentTypeId component) noexcept
    {
        m_commands.push(DeferredEntityCommand {
            .type = EntityCommandType::RemoveComponent,
            .deferredEntity = entity,
            .component = component
        });
    }

    void EntityCommands::materialize(World* world) noexcept
    {
        std::vector<Entity> materialized;
        materialized.resize(m_nextId);

        for (auto& deferredCommand : m_commands.deferredCommands())
        {
            if (deferredCommand.type == EntityCommandType::CreateEntity)
            {
                materialized[deferredCommand.deferredEntity.index] = world->createImmediate();
            }
            else
            {
                m_commands.push(EntityCommand{
                    .type = deferredCommand.type,
                    .entity = materialized[deferredCommand.deferredEntity.index],
                    .component = deferredCommand.component,
                    .data = deferredCommand.data
                    });
            }
        }
    }
}