#include <vector>

#include "litl-ecs/entity/entityCommands.hpp"
#include "litl-ecs/world.hpp"
#include "litl-core/constants.hpp"

namespace LITL::ECS
{
    constexpr uint32_t BlockSize = Constants::max_component_size * 16;

    struct LocalComponentDataBlock
    {
        void* copyInto(void* source, uint32_t size)
        {
            if (currOffset + size > BlockSize)
            {
                // not enough room
                return nullptr;
            }

            void* dest = &buffer[currOffset];

            memcpy(dest, source, size);
            currOffset += size;

            return dest;
        }

        alignas(Core::Constants::cache_line_size) std::byte buffer[BlockSize];
        uint32_t currOffset{ 0 };
    };

    struct LocalComponentData
    {
        LocalComponentData()
        {
            blocks.emplace_back();
        }

        void* copyInto(void* source, uint32_t size)
        {
            assert(source != nullptr);
            assert(size > 0);
            assert(size <= Constants::max_component_size);

            void* dest = blocks[currBlock].copyInto(source, size);

            if (dest == nullptr)
            {
                if (++currBlock >= blocks.size())
                {
                    blocks.emplace_back();
                }

                dest = blocks[currBlock].copyInto(source, size);
            }

            assert(dest != nullptr);

            return dest;
        }

        void reset()
        {
            currBlock = 0;

            for (auto& block : blocks)
            {
                block.currOffset = 0;
            }
        }

        std::vector<LocalComponentDataBlock> blocks{};
        uint32_t currBlock{ 0 };
    };

    struct EntityCommands::Impl
    {
        EntityCommandQueue commands{ };
        LocalComponentData localData{};
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
        m_pImpl->localData.reset();
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
            .component = component
        }, nullptr);
    }

    void EntityCommands::addComponent(DeferredEntity entity, ComponentTypeId component) noexcept
    {
        m_pImpl->commands.push(DeferredEntityCommand {
            .type = EntityCommandType::AddComponent,
            .deferredEntity = entity,
            .component = component
        }, nullptr);
    }

    void EntityCommands::addComponent(Entity entity, ComponentTypeId component, void* sharedData) noexcept
    {
        m_pImpl->commands.push(EntityCommand{
            .type = EntityCommandType::AddComponent,
            .entity = entity,
            .component = component
        }, sharedData);
    }

    void EntityCommands::addComponent(DeferredEntity entity, ComponentTypeId component, void* sharedData) noexcept
    {
        m_pImpl->commands.push(DeferredEntityCommand{
            .type = EntityCommandType::AddComponent,
            .deferredEntity = entity,
            .component = component
        }, sharedData);
    }


    void EntityCommands::addComponent(Entity entity, ComponentTypeId component, void* localData, uint32_t size) noexcept
    {
        m_pImpl->commands.push(EntityCommand{
            .type = EntityCommandType::AddComponent,
            .entity = entity,
            .component = component
        }, m_pImpl->localData.copyInto(localData, size));
    }

    void EntityCommands::addComponent(DeferredEntity entity, ComponentTypeId component, void* localData, uint32_t size) noexcept
    {
        m_pImpl->commands.push(DeferredEntityCommand{
            .type = EntityCommandType::AddComponent,
            .deferredEntity = entity,
            .component = component
        }, m_pImpl->localData.copyInto(localData, size));
    }

    void EntityCommands::removeComponent(Entity entity, ComponentTypeId component) noexcept
    {
        m_pImpl->commands.push(EntityCommand {
            .type = EntityCommandType::RemoveComponent,
            .entity = entity,
            .component = component
        });
    }

    void EntityCommands::removeComponent(DeferredEntity entity, ComponentTypeId component) noexcept
    {
        m_pImpl->commands.push(DeferredEntityCommand {
            .type = EntityCommandType::RemoveComponent,
            .deferredEntity = entity,
            .component = component
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
                    .component = deferredCommand.component,
                    .data = deferredCommand.data
                    });
            }
        }
    }
}