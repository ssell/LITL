#ifndef LITL_ENGINE_SCENE_NULL_PARTITION_H__
#define LITL_ENGINE_SCENE_NULL_PARTITION_H__

#include "litl-ecs/archetype/archetypeRegistry.hpp"
#include "litl-engine/scene/partition/scenePartition.hpp"

namespace litl
{
    /// <summary>
    /// A partition that performs no querying and simply passes all entities through.
    /// </summary>
    class NullPartition
    {
    public:

        NullPartition() 
        {
            m_newEntities.reserve(1024ull);
            m_entities.reserve(1024ull);
        }

        ~NullPartition() = default;

        void add(Entity entity, bounds::AABB bounds) noexcept 
        {
            m_newEntities.push_back(entity);
        }

        void remove(Entity entity) noexcept
        {
            // Check in m_entities first,
            for (size_t i = 0ull; i < m_entities.size(); ++i)
            {
                if (m_entities[i] == entity)
                {
                    m_entities[i] = m_entities.back();
                    m_entities.pop_back();
                    break;
                }
            }

            // Next, check in m_newEntities
            for (size_t i = 0ull; i < m_newEntities.size(); ++i)
            {
                if (m_newEntities[i] == entity)
                {
                    m_newEntities[i] = m_newEntities.back();
                    m_newEntities.pop_back();
                    break;
                }
            }
        }

        void preUpdate() noexcept
        {
            m_entities.insert(m_entities.end(), m_newEntities.begin(), m_newEntities.end());
            m_newEntities.clear();
        }

        void update(Entity entity, bounds::AABB bounds) noexcept 
        {
            // ... no action ...
        }

        void query(bounds::AABB bounds, std::vector<Entity>& entities) const noexcept
        {
            entities.insert(entities.begin(), m_entities.begin(), m_entities.end());        // return all
        }

        void query(bounds::AABB bounds, World& world, ComponentTypeId componentType, std::vector<Entity>& entities) const noexcept
        {
            for (auto entity : entities)
            {
                if (world.hasComponent(entity, componentType))
                {
                    entities.push_back(entity);
                }
            }
        }

        void query(bounds::Sphere bounds, std::vector<Entity>& entities) const noexcept
        {
            entities.insert(entities.begin(), m_entities.begin(), m_entities.end());        // return all
        }

        void query(bounds::Sphere bounds, World& world, ComponentTypeId componentType, std::vector<Entity>& entities) const noexcept
        {
            for (auto entity : m_entities)
            {
                if (world.hasComponent(entity, componentType))
                {
                    entities.push_back(entity);
                }
            }
        }

        void query(bounds::Frustum const& frustum, std::vector<Entity>& entities) const noexcept
        {
            entities.insert(entities.begin(), m_entities.begin(), m_entities.end());        // return all
        }

        void query(bounds::Frustum const& frustum, World& world, ComponentTypeId componentType, std::vector<Entity>& entities) const noexcept
        {
            for (auto entity : entities)
            {
                if (world.hasComponent(entity, componentType))
                {
                    entities.push_back(entity);
                }
            }
        }

    protected:

    private:

        /// <summary>
        /// Entities that have been added to the partition since the last call to update.
        /// These do not yet have world-space positions calculated for them and are so ineligible to be part of queries.
        /// </summary>
        std::vector<Entity> m_newEntities;

        /// <summary>
        /// Entities that have been present since the last update and have valid world-space positions.
        /// </summary>
        std::vector<Entity> m_entities;
    };

    static_assert(ScenePartition<NullPartition>);
}

#endif