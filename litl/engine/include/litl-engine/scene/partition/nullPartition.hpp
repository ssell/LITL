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

        NullPartition() {}
        ~NullPartition() {}

        void add(Entity entity, bounds::AABB bounds) noexcept 
        {
            m_entities.push_back(entity);
        }

        void remove(Entity entity) noexcept
        {
            for (auto i = 0; i < m_entities.size(); ++i)
            {
                if (m_entities[i] == entity)
                {
                    m_entities[i] = m_entities.back();
                    m_entities.pop_back();
                    break;
                }
            }
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
            for (auto entity : entities)
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

        std::vector<Entity> m_entities;
    };

    static_assert(ScenePartition<NullPartition>);
}

#endif