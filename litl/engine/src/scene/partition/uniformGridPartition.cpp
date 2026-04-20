#include "litl-core/assert.hpp"
#include "litl-engine/scene/partition/uniformGridPartition.hpp"

namespace litl
{
    struct GridCell
    {
        void add(Entity entity, bounds::AABB bounds) noexcept
        {

        }

        void remove(Entity entity) noexcept
        {

        }

        void query(bounds::AABB aabb, std::vector<Entity>& entities) const noexcept
        {

        }

        void query(bounds::Sphere sphere, std::vector<Entity>& entities) const noexcept
        {

        }

        void query(bounds::Frustum const& frustum, std::vector<Entity>& entities) const noexcept
        {

        }
    };

    struct UniformGridPartition::Impl
    {
        /// <summary>
        /// The options specified when creating this grid.
        /// </summary>
        UniformGridOptions options;

        /// <summary>
        /// All grid cells stored in row order.
        /// For example: [x0y0, x1y0, x2y0, x3y0, x0y1, x1y1, x2y1, x3y1, ...]
        /// </summary>
        std::vector<GridCell> cells;

        [[nodiscard]] uint32_t getGridLocalX(float worldX) const noexcept
        {
            return clamp(static_cast<uint32_t>(worldX), 0u, options.cellCount - 1u);
        }

        [[nodiscard]] uint32_t getGridLocalZ(float worldZ) const noexcept
        {
            return clamp(static_cast<uint32_t>(worldZ), 0u, options.cellCount - 1u);
        }

        [[nodiscard]] uint32_t getIndex(uint32_t cellX, uint32_t cellZ) const noexcept
        {
            return clamp(cellX, 0u, options.cellCount - 1u) + (clamp(cellZ, 0u, options.cellCount - 1u) * options.cellCount);
        }

        [[nodiscard]] uint32_t getIndex(vec3 point) const noexcept
        {
            const auto gridLocal = point - options.origin;
            const auto gridLocalX = getGridLocalX(point.x());
            const auto gridLocalZ = getGridLocalZ(point.z());
            return (gridLocalX + (gridLocalZ * options.cellCount));
        }
    };

    UniformGridPartition::UniformGridPartition(UniformGridOptions options)
    {
        LITL_FATAL_ASSERT_MSG(
            options.isValid(), 
            "UniformGridPartition must have both .cellSize and .cellCount that are power-of-twos and greater than one.");

        m_impl->options = options;
        m_impl->cells.resize(options.cellCount * options.cellCount);
    }

    UniformGridPartition::~UniformGridPartition()
    {

    }

    void UniformGridPartition::add(Entity entity, bounds::AABB bounds) noexcept
    {
        const auto gridIndex = m_impl->getIndex(bounds.center());
        m_impl->cells[gridIndex].add(entity, bounds);
    }

    void UniformGridPartition::remove(Entity entity, bounds::AABB bounds) noexcept
    {
        const auto gridIndex = m_impl->getIndex(bounds.center());
        m_impl->cells[gridIndex].remove(entity);
    }

    void UniformGridPartition::update(Entity entity, bounds::AABB prev, bounds::AABB curr) noexcept
    {
        const auto prevIndex = m_impl->getIndex(prev.center());
        const auto currIndex = m_impl->getIndex(curr.center());

        if (prevIndex != currIndex)
        {
            m_impl->cells[prevIndex].remove(entity);
            m_impl->cells[currIndex].add(entity, curr);
        }
    }

    void UniformGridPartition::query(bounds::AABB aabb, std::vector<Entity>& entities) const noexcept
    {
        const uint32_t startX = m_impl->getGridLocalX(aabb.min.x());
        const uint32_t endX = m_impl->getGridLocalX(aabb.max.x());

        const uint32_t startZ = m_impl->getGridLocalZ(aabb.min.z());
        const uint32_t endZ = m_impl->getGridLocalZ(aabb.max.z());

        for (uint32_t z = startZ; z <= endZ; ++z)
        {
            for (uint32_t x = startX; x <= endX; ++x)
            {
                m_impl->cells[x + (z * m_impl->options.cellCount)].query(aabb, entities);
            }
        }
    }

    void UniformGridPartition::query(bounds::Sphere sphere, std::vector<Entity>& entities) const noexcept
    {
        const uint32_t startX = m_impl->getGridLocalX(sphere.center.x() - sphere.radius);
        const uint32_t endX = m_impl->getGridLocalX(sphere.center.x() + sphere.radius);

        const uint32_t startZ = m_impl->getGridLocalZ(sphere.center.z() - sphere.radius);
        const uint32_t endZ = m_impl->getGridLocalZ(sphere.center.z() + sphere.radius);

        for (uint32_t z = startZ; z <= endZ; ++z)
        {
            for (uint32_t x = startX; x <= endX; ++x)
            {
                m_impl->cells[x + (z * m_impl->options.cellCount)].query(sphere, entities);
            }
        }
    }

    void UniformGridPartition::query(bounds::Frustum const& frustum, std::vector<Entity>& entities) const noexcept
    {

    }

    uint32_t UniformGridPartition::getCellSize() const noexcept
    {
        return m_impl->options.cellSize;
    }

    uint32_t UniformGridPartition::getCellCount() const noexcept
    {
        return m_impl->options.cellCount;
    }

    uint32_t UniformGridPartition::getWorldSize() const noexcept
    {
        return (m_impl->options.cellSize * m_impl->options.cellCount);
    }
}