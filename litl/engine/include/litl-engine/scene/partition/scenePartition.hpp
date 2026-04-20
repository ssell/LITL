#ifndef LITL_ENGINE_SCENE_PARTITION_H__
#define LITL_ENGINE_SCENE_PARTITION_H__

#include <concepts>
#include <vector>

#include "litl-ecs/entity/entity.hpp"
#include "litl-core/math/bounds/bounds.hpp"

namespace litl
{
    /// <summary>
    /// Defines the compile-time interface/contract that any scene partition implementation must abide by.
    /// </summary>
    template<typename T>
    concept ScenePartition = requires(T partition, Entity entity, bounds::AABB aabb, bounds::Sphere sphere, bounds::Frustum const& frustum, std::vector<Entity>& entities)
    {
        { partition.add(entity, aabb) } -> std::same_as<void>;
        { partition.remove(entity, aabb) } -> std::same_as<void>;
        { partition.update(entity, aabb, aabb) } -> std::same_as<void>;
        { partition.query(aabb, entities) } -> std::same_as<void>;
        { partition.query(sphere, entities) } -> std::same_as<void>;
        { partition.query(frustum, entities) } -> std::same_as<void>;
    };

    /*
        As per the ScenePartition concept, a partition must implement the following base structure:

        class ExamplePartition
        {
        public:

            void add(Entity entity, bounds::AABB bounds);
            void remove(Entity entity, bounds::AABB bounds);
            void update(Entity entity, bounds::AABB bounds, bounds::AABB bounds);
            void query(bounds::AABB bounds, std::vector<Entity>& entities);
            void query(bounds::Sphere bounds, std::vector<Entity>& entities);
            void query(bounds::Frustum const& frustum, std::vector<Entity>& entities);

        protected:

        private:
        };    
    */
}

#endif