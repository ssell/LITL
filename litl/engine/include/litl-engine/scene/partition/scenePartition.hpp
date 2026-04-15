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
    concept ScenePartition = requires(T partition, EntityId id , bounds::AABB aabb, bounds::Sphere sphere, bounds::Frustum frustum, std::vector<EntityId>& results )
    {
        { partition.add(id, aabb) } -> std::same_as<void>;
        { partition.remove(id) } -> std::same_as<void>;
        { partition.update(id, aabb) } -> std::same_as<void>;
        { partition.query(aabb, results) } -> std::same_as<void>;
        { partition.query(sphere, results) } -> std::same_as<void>;
        { partition.query(frustum, results) } -> std::same_as<void>;
    };

    /*
        As per the ScenePartition concept, a partition must implement the following base structure:

        class ExamplePartition
        {
        public:

            void add(EntityId id, BoundsAABB bounds);
            void remove(EntityId id);
            void update(EntityId id, BoundsAABB bounds);
            void query(bounds::AABB bounds, std::vector<EntityId>& found);
            void query(bounds::Sphere bounds, std::vector<EntityId>& found);
            void query(bounds::Frustum frustum, std::vector<EntityId>& found);

        protected:

        private:
        };    
    */
}

#endif