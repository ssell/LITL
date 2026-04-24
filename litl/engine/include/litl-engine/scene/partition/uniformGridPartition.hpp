#ifndef LITL_ENGINE_SCENE_UNIFORM_PARTITION_H__
#define LITL_ENGINE_SCENE_UNIFORM_PARTITION_H__

#include <optional>
#include <utility>

#include "litl-core/impl.hpp"
#include "litl-engine/scene/partition/scenePartition.hpp"
#include "litl-engine/scene/partition/partitionOptions.hpp"

namespace litl
{
    /// <summary>
    /// Information about an entity within the grid.
    /// </summary>
    struct UniformGridEntityInfo
    {
        Entity entity;
        bounds::AABB bounds;
        uint32_t cellIndex{ 0 };
        bool isOversized{ false };
    };

    class UniformGridPartition
    {
    public:

        UniformGridPartition();
        UniformGridPartition(UniformGridOptions const& options);
        UniformGridPartition(UniformGridPartition&) = delete;
        UniformGridPartition& operator=(UniformGridPartition const&) = delete;

        ~UniformGridPartition();

        /// <summary>
        /// Adds the entity to the grid.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="aabb"></param>
        void add(Entity entity, bounds::AABB aabb) noexcept;

        /// <summary>
        /// Removes the entity from the grid.
        /// </summary>
        /// <param name="entity"></param>
        void remove(Entity entity) noexcept;

        /// <summary>
        /// Updates the bounds of the entity in the grid.
        /// As a result of this, the entity may be moved to a different cell if it has moved sufficiently enough.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="bounds"></param>
        void update(Entity entity, bounds::AABB bounds) noexcept;

        /// <summary>
        /// Queries for all entities in the grid that intersect the specified AABB.
        /// </summary>
        /// <param name="aabb"></param>
        /// <param name="entities"></param>
        void query(bounds::AABB aabb, std::vector<Entity>& entities) const noexcept;

        /// <summary>
        /// Queries for all entities in the grid that intersect the specified Sphere.
        /// </summary>
        /// <param name="sphere"></param>
        /// <param name="entities"></param>
        void query(bounds::Sphere sphere, std::vector<Entity>& entities) const noexcept;

        /// <summary>
        /// Queries for all entities in the grid that intersect the specified Frustum.
        /// </summary>
        /// <param name="frustum"></param>
        /// <param name="entities"></param>
        void query(bounds::Frustum const& frustum, std::vector<Entity>& entities) const noexcept;

        /// <summary>
        /// Returns the size along each dimensions for an individual cell.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] uint32_t getCellSize() const noexcept;

        /// <summary>
        /// Returns the number of cells along each dimension.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] uint32_t getCellCount() const noexcept;

        /// <summary>
        /// Returns the total length along each dimension (cell size * cell count).
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] uint32_t getWorldSize() const noexcept;

        /// <summary>
        /// Returns the number of entities in the specified cell.
        /// </summary>
        /// <param name="cellX"></param>
        /// <param name="cellZ"></param>
        /// <returns></returns>
        [[nodiscard]] uint32_t getCellPopulation(uint32_t cellX, uint32_t cellZ) const noexcept;

        /// <summary>
        /// Returns the number of entities classified as oversized.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] uint32_t getOversizedCellPopulation() const noexcept;

        /// <summary>
        /// Returns the number of entities in the grid.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] uint32_t getGridPopulation() const noexcept;

        /// <summary>
        /// Returns the cell which maps to the given world position.
        /// </summary>
        /// <param name="worldPos"></param>
        /// <returns></returns>
        [[nodiscard]] std::pair<uint32_t, uint32_t> getCellIndex(vec3 worldPos) const noexcept;

        /// <summary>
        /// Returns information about an entity in the grid, if it is in the grid.
        /// </summary>
        /// <param name="entityId"></param>
        /// <returns></returns>
        [[nodiscard]] std::optional<UniformGridEntityInfo> getEntityInfo(EntityId entityId) const noexcept;

    protected:

    private:

        void configure(UniformGridOptions const& options) noexcept;

        struct Impl; ImplPtr<Impl, 256> m_impl;
    };

    static_assert(ScenePartition<UniformGridPartition>);
}

#endif