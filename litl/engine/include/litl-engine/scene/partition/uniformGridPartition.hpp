#ifndef LITL_ENGINE_SCENE_UNIFORM_PARTITION_H__
#define LITL_ENGINE_SCENE_UNIFORM_PARTITION_H__

#include "litl-core/impl.hpp"
#include "litl-core/math/common.hpp"
#include "litl-engine/scene/partition/scenePartition.hpp"

namespace litl
{
    struct UniformGridOptions
    {
        /// <summary>
        /// The dimensions of each cell, in world units.
        /// Must be a positive power of two value.
        /// </summary>
        uint32_t cellSize{ 32 };

        /// <summary>
        /// The number of cells along each dimension.
        /// </summary>
        uint32_t cellCount{ 32 };

        [[nodiscard]] static constexpr UniformGridOptions fromWorldSize(uint32_t worldDimensionsXY, uint32_t cellSize = 32)
        {
            cellSize = roundUpToPow2(cellSize);
            worldDimensionsXY = roundUpToPow2(worldDimensionsXY);

            if (worldDimensionsXY <= cellSize)
            {
                worldDimensionsXY *= 2;
            }

            return UniformGridOptions{
                .cellSize = cellSize,
                .cellCount = worldDimensionsXY / cellSize
            };
        }
    };

    class UniformGridPartition
    {
    public:

        UniformGridPartition(UniformGridOptions options);
        ~UniformGridPartition();

        UniformGridPartition(UniformGridPartition const&) = delete;
        UniformGridPartition& operator=(UniformGridPartition const&) = delete;

        void add(EntityId id, bounds::AABB aabb) noexcept;
        void remove(EntityId id) noexcept;
        void update(EntityId id, bounds::AABB aabb) noexcept;
        void query(bounds::AABB aabb, std::vector<EntityId>& entities) const noexcept;
        void query(bounds::Sphere sphere, std::vector<EntityId>& entities) const noexcept;
        void query(bounds::Frustum const& frustum, std::vector<EntityId>& entities) const noexcept;

    protected:

    private:

        UniformGridOptions m_options;
        struct Impl; ImplPtr<Impl, 64> m_impl;
    };

    static_assert(ScenePartition<UniformGridPartition>);
}

#endif