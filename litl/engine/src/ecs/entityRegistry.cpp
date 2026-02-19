#include <atomic>
#include <mutex>

#include "litl-core/containers/pagedVector.hpp"
#include "litl-engine/ecs/entityRegistry.hpp"

namespace LITL::Engine::ECS
{
    struct EntityRegistryState
    {
        Core::PagedVector<EntityRecord> entityRecords;
        std::vector<uint32_t> deadEntities;
    };

    namespace
    {
        EntityRegistryState& instance()
        {
            static EntityRegistryState state;
            return state;
        }
    }

    Entity EntityRegistry::create() noexcept
    {
        EntityRegistryState& registry = instance();
        uint32_t index = static_cast<uint32_t>(registry.entityRecords.size());

        if (!registry.deadEntities.empty())
        {
            index = registry.deadEntities.back();
            registry.deadEntities.pop_back();
        }
        else
        {
            registry.entityRecords.emplace_back(EntityRecord{
                .entity = Entity {
                    .index = index,
                    .version = 1
                },
                .index = index, 
                .archetype = nullptr
            });
        }

        return registry.entityRecords[index].entity;
    }

    std::vector<Entity> EntityRegistry::createMany(uint32_t count) noexcept
    {
        std::vector<Entity> result;
        result.reserve(count);

        EntityRegistryState& registry = instance();

        // For simplicity, reuse only if we have enough dead to satisfy the count
        if (registry.deadEntities.size() >= count)
        {
            for (auto i = static_cast<uint32_t>(0); i < count; ++i)
            {
                auto index = registry.deadEntities.back();
                registry.deadEntities.pop_back();
                result.emplace_back(registry.entityRecords[index].entity);
            }
        }
        else
        {
            const uint32_t startIndex = registry.entityRecords.size();
            const uint32_t endIndex = startIndex + count;

            for (auto index = startIndex; index < endIndex; ++index)
            {
                Entity entity = {
                    .index = index,
                    .version = 1
                };

                registry.entityRecords.emplace_back(EntityRecord{
                    .entity = entity,
                    .index = index,
                    .archetype = nullptr
                    });

                result.emplace_back(entity);
            }
        }
        return result;
    }

    void EntityRegistry::destroy(Entity entity) noexcept
    {
        auto& record = getRecord(entity);

        if (record.entity.version == entity.version)
        {
            // ... todo remove from archetype ...

            record.entity.version++;    // increment on death to invalidate any lingering handles
            instance().deadEntities.emplace_back(record.index);
        }
    }

    void EntityRegistry::destroyMany(std::initializer_list<Entity> entities) noexcept
    {
        for (auto entity : entities)
        {
            destroy(entity);
        }
    }

    void EntityRegistry::destroyMany(std::span<Entity const> entities) noexcept
    {
        for (auto i = 0; i < entities.size(); ++i)
        {
            destroy(entities[i]);
        }
    }

    EntityRecord& EntityRegistry::getRecord(Entity entity) noexcept
    {
        return instance().entityRecords[entity.index];
    }

    bool EntityRegistry::isAlive(Entity entity) noexcept
    {
        return instance().entityRecords[entity.index].entity.version == entity.version;
    }

    void EntityRegistry::clear() noexcept
    {
        instance().deadEntities.clear();
        instance().entityRecords.clear();
    }
}