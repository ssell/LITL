#ifndef LITL_ENGINE_SCENE_PARTITION_H__
#define LITL_ENGINE_SCENE_PARTITION_H__

#include <concepts>
#include "litl-ecs/entity/entity.hpp"

namespace litl
{
    /// <summary>
    /// Defines the compile-time interface/contract that any scene partition implementation must abide by.
    /// </summary>
    template<typename T>
    concept ScenePartition = requires(T partition, EntityId id /*, AABB bounds, Frustum frustum, std::vector<EntityId> results */)
    {
        { partition.add(id/*, bounds*/) } -> std::same_as<void>;
        { partition.remove(id) } -> std::same_as<void>;
        { partition.update(id/*, bounds*/) } -> std::same_as<void>;
        /* todo query bounds aabb */
        /* todo query bounds sphere */
        /* todo query frustum */
    };

    /*
        As per the ScenePartition concept, a partition must implement the following base structure:

        class ExamplePartition
        {
        public:

            void add(EntityId id, BoundsAABB bounds);
            void remove(EntityId id);
            void update(EntityId id, BoundsAABB bounds);
            void query(BoundsAABB bounds, std::vector<EntityId>& found);
            void query(BoundsSphere bounds, std::vector<EntityId>& found);
            void query(Frustum frustum, std::vector<EntityId>& found);

        protected:

        private:
        };    
    */
}

#endif