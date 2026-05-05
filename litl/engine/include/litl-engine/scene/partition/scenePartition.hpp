#ifndef LITL_ENGINE_SCENE_PARTITION_H__
#define LITL_ENGINE_SCENE_PARTITION_H__

#include <cstdint>
#include <concepts>
#include <vector>

#include "litl-ecs/entity/entity.hpp"
#include "litl-core/math/bounds.hpp"

namespace litl
{
    /// <summary>
    /// Defines the compile-time interface/contract that any scene partition implementation must abide by.
    /// 
    /// All positions, bounds, and queries are expected to be in world-space.
    /// </summary>
    template<typename T>
    concept ScenePartition = requires(
        T partition,
        T const cpartition,
        Entity entity,
        bounds::AABB const& bounds,         // <-- const& avoids copy construction (doesn't have to be in the actual impl, needed for the compile-time check)
        bounds::Sphere const& sphere,       // <-- const& avoids copy construction (doesn't have to be in the actual impl, needed for the compile-time check)
        bounds::Frustum const& frustum,
        std::vector<Entity>& entities)
    {
        { partition.add(entity, bounds) } noexcept -> std::same_as<void>;
        { partition.remove(entity) } noexcept -> std::same_as<void>;
        { partition.update(entity, bounds) } noexcept -> std::same_as<void>;

        { cpartition.query(bounds, entities) } noexcept -> std::same_as<void>;      // const
        { cpartition.query(sphere, entities) } noexcept -> std::same_as<void>;      // const
        { cpartition.query(frustum, entities) } noexcept -> std::same_as<void>;     // const
    };

    /*
        As per the ScenePartition concept, a partition must implement the following base structure:

        class ExamplePartition
        {
        public:

            void add(Entity entity, bounds::AABB bounds) noexcept;
            void remove(Entity entity) noexcept;
            void update(Entity entity, bounds::AABB bounds) noexcept;

            void query(bounds::AABB bounds, std::vector<Entity>& entities) const noexcept;
            void query(bounds::Sphere bounds, std::vector<Entity>& entities) const noexcept;
            void query(bounds::Frustum const& frustum, std::vector<Entity>& entities) const noexcept;

        protected:

        private:
        };    
    */

    enum class ScenePartitionType : uint32_t
    {
        UniformGrid = 0u
    };
}

#endif