#include <vector>

#include "litl-core/containers/memoryArena.hpp"
#include "litl-ecs/entity/entityCommands.hpp"
#include "litl-ecs/world.hpp"

namespace LITL::ECS
{
    constexpr uint32_t BlockSize = Constants::max_component_size * 16;

    struct EntityCommands::Impl
    {
        EntityCommandQueue commands{ };
        Core::MemoryArena<BlockSize, 128> localData{};
        uint32_t nextId{ 0 };
    };

    EntityCommands::EntityCommands()
        : m_pImpl(std::make_unique<EntityCommands::Impl>())
    {

    }

    EntityCommands::~EntityCommands()
    {

    }

    void EntityCommands::reset() noexcept
    {
        m_pImpl->nextId = 0;
        m_pImpl->commands.reset();
        m_pImpl->localData.resetShrinkAuto();
    }

    size_t EntityCommands::actionableCommandCount() const noexcept
    {
        return m_pImpl->commands.actionableCommandCount();
    }

    void EntityCommands::extractCommands(World* world, std::vector<EntityCommand>& commands, size_t offset) noexcept
    {
        assert(world != nullptr);
        assert(commands.size() >= (m_pImpl->commands.count() + offset));

        materialize(world);

        auto count = m_pImpl->commands.count();

        for (auto i = offset; i < (offset + count); ++i)
        {
            commands[i] = m_pImpl->commands.next().value();
        }
    }

    DeferredEntity EntityCommands::createEntity() noexcept
    {
        DeferredEntity entity = { static_cast<uint32_t>(m_pImpl->nextId++) };

        m_pImpl->commands.push(DeferredEntityCommand {
            .type = EntityCommandType::CreateEntity,
            .deferredEntity = entity
        });

        return entity;
    }

    void EntityCommands::destroyEntity(Entity entity) noexcept
    {
        m_pImpl->commands.push(EntityCommand {
            .type = EntityCommandType::DestroyEntity,
            .entity = entity
        });
    }

    void EntityCommands::destroyEntity(DeferredEntity entity) noexcept
    {
        m_pImpl->commands.push(DeferredEntityCommand {
            .type = EntityCommandType::DestroyEntity,
            .deferredEntity = entity
        });
    }

    void EntityCommands::addComponent(Entity entity, ComponentTypeId component) noexcept
    {
        m_pImpl->commands.push(EntityCommand {
            .type = EntityCommandType::AddComponent,
            .entity = entity,
            .componentInfo = ComponentCommandInfo {
                .component = component
            }
        }, nullptr);
    }

    void EntityCommands::addComponent(DeferredEntity entity, ComponentTypeId component) noexcept
    {
        m_pImpl->commands.push(DeferredEntityCommand {
            .type = EntityCommandType::AddComponent,
            .deferredEntity = entity,
            .componentInfo = ComponentCommandInfo {
                .component = component
            }
        }, nullptr);
    }

    void EntityCommands::addComponent(Entity entity, ComponentTypeId component, void* sharedData) noexcept
    {
        m_pImpl->commands.push(EntityCommand{
            .type = EntityCommandType::AddComponent,
            .entity = entity,
            .componentInfo = ComponentCommandInfo {
                .component = component
            }
        }, sharedData);
    }

    void EntityCommands::addComponent(DeferredEntity entity, ComponentTypeId component, void* sharedData) noexcept
    {
        m_pImpl->commands.push(DeferredEntityCommand{
            .type = EntityCommandType::AddComponent,
            .deferredEntity = entity,
            .componentInfo = ComponentCommandInfo {
                .component = component
            }
        }, sharedData);
    }


    void EntityCommands::addComponent(Entity entity, ComponentTypeId component, void* localData, size_t size, size_t alignment) noexcept
    {
        m_pImpl->commands.push(EntityCommand{
            .type = EntityCommandType::AddComponent,
            .entity = entity,
            .componentInfo = ComponentCommandInfo {
                .component = component
            }
        }, m_pImpl->localData.insert(localData, size, alignment));
    }

    void EntityCommands::addComponent(DeferredEntity entity, ComponentTypeId component, void* localData, size_t size, size_t alignment) noexcept
    {
        m_pImpl->commands.push(DeferredEntityCommand{
            .type = EntityCommandType::AddComponent,
            .deferredEntity = entity,
            .componentInfo = ComponentCommandInfo {
                .component = component
            }
        }, m_pImpl->localData.insert(localData, size, alignment));
    }

    void EntityCommands::removeComponent(Entity entity, ComponentTypeId component) noexcept
    {
        m_pImpl->commands.push(EntityCommand {
            .type = EntityCommandType::RemoveComponent,
            .entity = entity,
            .componentInfo = ComponentCommandInfo {
                .component = component
            }
        });
    }

    void EntityCommands::removeComponent(DeferredEntity entity, ComponentTypeId component) noexcept
    {
        m_pImpl->commands.push(DeferredEntityCommand {
            .type = EntityCommandType::RemoveComponent,
            .deferredEntity = entity,
            .componentInfo = ComponentCommandInfo {
                .component = component
            }
        });
    }

    void EntityCommands::materialize(World* world) noexcept
    {
        std::vector<Entity> materialized;
        materialized.resize(m_pImpl->nextId);

        for (auto& deferredCommand : m_pImpl->commands.deferredCommands())
        {
            if (deferredCommand.type == EntityCommandType::CreateEntity)
            {
                materialized[deferredCommand.deferredEntity.index] = world->createImmediate();
            }
            else
            {
                m_pImpl->commands.push(EntityCommand{
                    .type = deferredCommand.type,
                    .entity = materialized[deferredCommand.deferredEntity.index],
                    .componentInfo = deferredCommand.componentInfo
                });
            }
        }
    }
}