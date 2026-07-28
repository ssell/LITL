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

                    m_bounds[i] = m_bounds.back();
                    m_bounds.pop_back();

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
            for (auto entity : m_newEntities)
            {
                m_entities.push_back(entity);
                m_bounds.push_back({});
            }

            m_newEntities.clear();
        }

        void update(Entity entity, bounds::AABB bounds) noexcept 
        {
            for (size_t i = 0ull; i < m_entities.size(); ++i)
            {
                if (m_entities[i] == entity)
                {
                    m_bounds[i] = bounds;
                    break;
                }
            }
        }

        void query(bounds::AABB bounds, std::vector<PartitionQueryResult>& entities) const noexcept
        {
            auto queryCenter = bounds.center();

            for (size_t i = 0ull; i < m_entities.size(); ++i)
            {
                entities.push_back(PartitionQueryResult{
                    .entity = m_entities[i],
                    .worldPosition = m_bounds[i].center(),
                    .distanceSquared = m_bounds[i].center().distanceSqTo(queryCenter)
                });
            }
        }

        void query(bounds::AABB bounds, World& world, ComponentTypeId componentType, std::vector<PartitionQueryResult>& entities) const noexcept
        {
            auto queryCenter = bounds.center();

            for (size_t i = 0ull; i < m_entities.size(); ++i)
            {
                if (world.hasComponent(m_entities[i], componentType))
                {
                    entities.push_back(PartitionQueryResult{
                        .entity = m_entities[i],
                        .worldPosition = m_bounds[i].center(),
                        .distanceSquared = m_bounds[i].center().distanceSqTo(queryCenter)
                    });
                }
            }
        }

        void query(bounds::Sphere bounds, std::vector<PartitionQueryResult>& entities) const noexcept
        {
            for (size_t i = 0ull; i < m_entities.size(); ++i)
            {
                entities.push_back(PartitionQueryResult{
                    .entity = m_entities[i],
                    .worldPosition = m_bounds[i].center(),
                    .distanceSquared = m_bounds[i].center().distanceSqTo(bounds.center)
                });
            }
        }

        void query(bounds::Sphere bounds, World& world, ComponentTypeId componentType, std::vector<PartitionQueryResult>& entities) const noexcept
        {
            for (size_t i = 0ull; i < m_entities.size(); ++i)
            {
                if (world.hasComponent(m_entities[i], componentType))
                {
                    entities.push_back(PartitionQueryResult{
                        .entity = m_entities[i],
                        .worldPosition = m_bounds[i].center(),
                        .distanceSquared = m_bounds[i].center().distanceSqTo(bounds.center)
                    });
                }
            }
        }

        void query(bounds::Frustum const& frustum, std::vector<PartitionQueryResult>& entities) const noexcept
        {
            auto queryCenter = frustum.getOrigin();

            for (size_t i = 0ull; i < m_entities.size(); ++i)
            {
                entities.push_back(PartitionQueryResult{
                    .entity = m_entities[i],
                    .worldPosition = m_bounds[i].center(),
                    .distanceSquared = queryCenter.distanceSqTo(m_bounds[i].center())
                });
            }
        }

        void query(bounds::Frustum const& frustum, World& world, ComponentTypeId componentType, std::vector<PartitionQueryResult>& entities) const noexcept
        {
            auto queryCenter = frustum.getOrigin();

            for (size_t i = 0ull; i < m_entities.size(); ++i)
            {
                if (world.hasComponent(m_entities[i], componentType))
                {
                    entities.push_back(PartitionQueryResult{
                        .entity = m_entities[i],
                        .worldPosition = m_bounds[i].center(),
                        .distanceSquared = queryCenter.distanceSqTo(m_bounds[i].center())
                    });
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

        /// <summary>
        /// Entity bounds.
        /// </summary>
        std::vector<bounds::AABB> m_bounds;
    };

    static_assert(ScenePartition<NullPartition>);
}

#endif