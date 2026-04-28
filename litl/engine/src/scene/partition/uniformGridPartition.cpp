#include "litl-core/assert.hpp"
#include "litl-core/math/bounds.hpp"
#include "litl-engine/scene/partition/uniformGridPartition.hpp"

namespace litl
{
    /// <summary>
    /// Represents a single cell within the grid.
    /// Each cell contains a list of entities and their AABB bounds.
    /// </summary>
    struct GridCell
    {
        /// <summary>
        /// All entities in the cell.
        /// </summary>
        std::vector<Entity> entities;

        /// <summary>
        /// The bounds for each entity in the cell.
        /// </summary>
        std::vector<bounds::AABB> entityBounds;

        GridCell(float x, float z, float size, float yMin, float yMax)
            : cellBounds(bounds::AABB::fromMinMax(vec3{ x, yMin, z }, vec3{ x + size, yMax, z + size }))
        {

        }

        /// <summary>
        /// Queries for all entities in the cell that intersect the specified AABB.
        /// </summary>
        /// <param name="aabb"></param>
        /// <param name="outEntities"></param>
        void query(bounds::AABB aabb, std::vector<Entity>& outEntities) const noexcept
        {
            const auto intersection = bounds::classify(aabb, cellBounds);

            switch (intersection)
            {
                // The cell is completely inside the AABB, so add all
            case bounds::IntersectionType::Inside:
                addAllTo(outEntities);
                break;

                // The cell intersects the AABB, so add some
            case bounds::IntersectionType::Intersects:
                for (uint32_t i = 0u; i < count(); ++i)
                {
                    if (bounds::intersects(aabb, entityBounds[i]))      // intersects returns true for both true intersection (straddle) and containment
                    {
                        outEntities.push_back(entities[i]);
                    }
                }
                break;

                // The cell is completely outside the AABB, so add none
            case bounds::IntersectionType::Outside:
            default:
                break;
            }
        }

        /// <summary>
        /// Queries for all entities in the cell that intersect the specified Sphere.
        /// </summary>
        /// <param name="sphere"></param>
        /// <param name="outEntities"></param>
        void query(bounds::Sphere sphere, std::vector<Entity>& outEntities) const noexcept
        {
            const auto intersection = bounds::classify(sphere, cellBounds);

            switch (intersection)
            {
                // The cell is completely inside the Sphere, so add all
            case bounds::IntersectionType::Inside:
                addAllTo(outEntities);
                break;

                // The cell intersects the Sphere, so add some
            case bounds::IntersectionType::Intersects:
                for (uint32_t i = 0u; i < count(); ++i)
                {
                    if (bounds::intersects(sphere, entityBounds[i]))        // intersects returns true for both true intersection (straddle) and containment
                    {
                        outEntities.push_back(entities[i]);
                    }
                }
                break;

                // The cell is completely outside the Sphere, so add none
            case bounds::IntersectionType::Outside:
            default:
                break;
            }
        }

        /// <summary>
        /// Queries for all entities in the cell that intersect the specified Frustum.
        /// </summary>
        /// <param name="frustum"></param>
        /// <param name="outEntities"></param>
        void query(bounds::Frustum const& frustum, std::vector<Entity>& outEntities) const noexcept
        {
            const auto classification = bounds::classify(frustum, cellBounds);

            switch (classification.type())
            {
                // The cell is completely inside the Frustum, so add all
            case bounds::IntersectionType::Inside:
                addAllTo(outEntities);
                break;

                // The cell intersects the Frustum, so add some
            case bounds::IntersectionType::Intersects:
                for (uint32_t i = 0u; i < count(); ++i)
                {
                    if (bounds::intersects(frustum, entityBounds[i]))       // intersects returns true for both true intersection (straddle) and containment
                    {
                        outEntities.push_back(entities[i]);
                    }
                }
                break;

                // The cell is completely outside the Frustum, so add none
            case bounds::IntersectionType::Outside:
            default:
                break;
            }
        }

        /// <summary>
        /// Returns the number of entities within the cell.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] uint32_t count() const noexcept
        {
            return static_cast<uint32_t>(entities.size());
        }

    private:

        /// <summary>
        /// Adds all entities in the cell to the vector.
        /// </summary>
        /// <param name="entities"></param>
        void addAllTo(std::vector<Entity>& outEntities) const noexcept
        {
            outEntities.insert(outEntities.end(), entities.begin(), entities.end());
        }

        /// <summary>
        /// The AABB bounds of the cell.
        /// </summary>
        const bounds::AABB cellBounds;
    };

    struct UniformGridPartition::Impl
    {
        /// <summary>
        /// The options specified when creating this grid.
        /// </summary>
        UniformGridOptions options;

        /// <summary>
        /// The squared radius/half-extents threshold at which an entity is considered oversized.
        /// </summary>
        float oversizedEntityThreshold{ 1.0f };

        /// <summary>
        /// All grid cells stored in row order.
        /// For example: [x0y0, x1y0, x2y0, x3y0, x0y1, x1y1, x2y1, x3y1, ...]
        /// </summary>
        std::vector<GridCell> cells;

        /// <summary>
        /// Maps an entity to the GridCell it is in.
        /// </summary>
        std::unordered_map<EntityId, uint32_t> entityToCell;

        /// <summary>
        /// Maps an entity to the slot within the GridCell it s in.
        /// </summary>
        std::unordered_map<EntityId, uint32_t> entityToCellSlot;

        /// <summary>
        /// Returns the index of the cell along the x-axis that the specified world position-x fits into.
        /// </summary>
        /// <param name="worldX"></param>
        /// <returns></returns>
        [[nodiscard]] uint32_t getCellIndexX(float worldX) const noexcept
        {
            const auto gridSize = static_cast<float>(options.cellCount * options.cellSize);
            const auto localX = worldX - options.origin.x();
            const auto localClampedX = clamp(localX, 0.0f, gridSize);
            const auto cellIndex = static_cast<uint32_t>(localClampedX / static_cast<float>(options.cellSize));

            return min(cellIndex, options.cellCount - 1);
        }

        /// <summary>
        /// Returns the index of the cell along the z-axis that the specified world position-z fits into.
        /// </summary>
        /// <param name="worldZ"></param>
        /// <returns></returns>
        [[nodiscard]] uint32_t getCellIndexZ(float worldZ) const noexcept
        {
            const auto gridSize = static_cast<float>(options.cellCount * options.cellSize);
            const auto localZ = worldZ - options.origin.z();
            const auto localClampedZ = clamp(localZ, 0.0f, gridSize);
            const auto cellIndex = static_cast<uint32_t>(localClampedZ / static_cast<float>(options.cellSize));

            return min(cellIndex, options.cellCount - 1);
        }

        /// <summary>
        /// Returns the linear cell index for the given 2D cell index.
        /// </summary>
        /// <param name="cellX"></param>
        /// <param name="cellZ"></param>
        /// <returns></returns>
        [[nodiscard]] uint32_t getIndex(uint32_t cellX, uint32_t cellZ) const noexcept
        {
            return clamp(cellX, 0u, options.cellCount - 1u) + (clamp(cellZ, 0u, options.cellCount - 1u) * options.cellCount);
        }

        /// <summary>
        /// Returns the index of the cell that contains (or clamps) the specified point.
        /// </summary>
        /// <param name="point"></param>
        /// <returns></returns>
        [[nodiscard]] uint32_t getIndexForPoint(vec3 point) const noexcept
        {
            const auto gridLocalX = getCellIndexX(point.x());
            const auto gridLocalZ = getCellIndexZ(point.z());
            return (gridLocalX + (gridLocalZ * options.cellCount));
        }

        /// <summary>
        /// Returns the index of the cell that the bounds would go into.
        /// If the bounds is considered oversized, then the oversized cell index is returned.
        /// </summary>
        /// <param name="bounds"></param>
        /// <returns></returns>
        [[nodiscard]] uint32_t getIndexForBounds(bounds::AABB bounds) const noexcept
        {
            if (isOversized(bounds))
            {
                return getOversizedCellIndex();
            }
            else
            {
                return getIndexForPoint(bounds.center());
            }
        }

        /// <summary>
        /// Adds the entity to the grid.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="bounds"></param>
        void add(Entity entity, bounds::AABB bounds) noexcept
        {
            LITL_ASSERT_MSG(entityToCell.find(entity.index) == entityToCell.end(), "Attempting to add Entity to UniformGridPartition whose index is already tracked.", );
            addEntityTo(entity, bounds, getIndexForBounds(bounds));
        }

        /// <summary>
        /// Removes the entity from the grid.
        /// </summary>
        /// <param name="entity"></param>
        void remove(Entity entity) noexcept
        {
            const auto findEntity = entityToCell.find(entity.index);

            if (findEntity == entityToCell.end())
            {
                return;
            }

            const auto cellIndex = findEntity->second;
            const auto cellSlot = entityToCellSlot.find(entity.index)->second;

            auto& cell = cells[cellIndex];

            if (cell.entities[cellSlot].version > entity.version)
            {
                // The entity tracked is newer than the one being requested to remove.
                return;
            }

            removeEntityFrom(entity, cellIndex, cellSlot);
        }

        /// <summary>
        /// Updates the bounds of the entity in the grid.
        /// As a result of this, the entity may be moved to a different cell if it has moved sufficiently enough.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="bounds"></param>
        void update(Entity entity, bounds::AABB bounds) noexcept
        {
            const auto findEntity = entityToCell.find(entity.index);

            if (findEntity == entityToCell.end())
            {
                return;
            }

            const auto prevEntityIndex = findEntity->second;
            const auto prevEntitySlot = entityToCellSlot.find(entity.index)->second;
            const auto currEntityIndex = getIndexForBounds(bounds);

            if (currEntityIndex == prevEntityIndex)
            {
                // The entity is in the same cell. Just update the bounds.
                cells[prevEntityIndex].entityBounds[prevEntitySlot] = bounds;
            }
            else
            {
                // The entity has moved cells. Remove from its current cell and add to the new one.
                removeEntityFrom(entity, prevEntityIndex, prevEntitySlot);
                addEntityTo(entity, bounds, currEntityIndex);
            }
        }

        /// <summary>
        /// Queries for all entities in the grid that intersect the specified AABB.
        /// </summary>
        /// <param name="aabb"></param>
        /// <param name="entities"></param>
        void query(bounds::AABB aabb, std::vector<Entity>& entities) const noexcept
        {
            const uint32_t startX = getCellIndexX(aabb.min.x());
            const uint32_t endX = getCellIndexX(aabb.max.x());

            const uint32_t startZ = getCellIndexZ(aabb.min.z());
            const uint32_t endZ = getCellIndexZ(aabb.max.z());

            for (uint32_t z = startZ; z <= endZ; ++z)
            {
                for (uint32_t x = startX; x <= endX; ++x)
                {
                    cells[x + (z * options.cellCount)].query(aabb, entities);
                }
            }

            getOversizedCell().query(aabb, entities);
        }

        /// <summary>
        /// Queries for all entities in the grid that intersect the specified Sphere.
        /// </summary>
        /// <param name="sphere"></param>
        /// <param name="entities"></param>
        void query(bounds::Sphere sphere, std::vector<Entity>& entities) const noexcept
        {
            const uint32_t startX = getCellIndexX(sphere.center.x() - sphere.radius);
            const uint32_t endX = getCellIndexX(sphere.center.x() + sphere.radius);

            const uint32_t startZ = getCellIndexZ(sphere.center.z() - sphere.radius);
            const uint32_t endZ = getCellIndexZ(sphere.center.z() + sphere.radius);

            for (uint32_t z = startZ; z <= endZ; ++z)
            {
                for (uint32_t x = startX; x <= endX; ++x)
                {
                    cells[x + (z * options.cellCount)].query(sphere, entities);
                }
            }

            getOversizedCell().query(sphere, entities);
        }

        /// <summary>
        /// Queries for all entities in the grid that intersect the specified Frustum.
        /// </summary>
        /// <param name="frustum"></param>
        /// <param name="entities"></param>
        void query(bounds::Frustum const& frustum, std::vector<Entity>& entities) const noexcept
        {
            const bounds::AABB frustumAABB = bounds::computeAABB(frustum);

            // same logic as in query(aabb, entities)
            const uint32_t startX = getCellIndexX(frustumAABB.min.x());
            const uint32_t endX = getCellIndexX(frustumAABB.max.x());

            const uint32_t startZ = getCellIndexZ(frustumAABB.min.z());
            const uint32_t endZ = getCellIndexZ(frustumAABB.max.z());

            for (uint32_t z = startZ; z <= endZ; ++z)
            {
                for (uint32_t x = startX; x <= endX; ++x)
                {
                    cells[x + (z * options.cellCount)].query(frustum, entities);
                }
            }

            getOversizedCell().query(frustum, entities);
        }

        /// <summary>
        /// Determine if the entity is considered oversized based on it's size across the XZ plane.
        /// </summary>
        /// <param name="bounds"></param>
        /// <returns></returns>
        bool isOversized(bounds::AABB bounds) const noexcept
        {
            const float xHalfExtents = (bounds.max.x() - bounds.min.x()) * 0.5f;
            const float xHalfExtentsSq = xHalfExtents * xHalfExtents;

            const float zHalfExtents = (bounds.max.z() - bounds.min.z()) * 0.5f;
            const float zHalfExtentsSq = zHalfExtents * zHalfExtents;

            return (xHalfExtentsSq >= oversizedEntityThreshold) || (zHalfExtentsSq >= oversizedEntityThreshold);
        }

        /// <summary>
        /// Returns the index of the extra "overflow" cell that contains oversized entities.
        /// </summary>
        /// <returns></returns>
        uint32_t getOversizedCellIndex() const noexcept
        {
            return cells.size() - 1;
        }

        /// <summary>
        /// Returns the extra "overflow" cell that contains oversized entities.
        /// </summary>
        /// <returns></returns>
        GridCell const& getOversizedCell() const noexcept
        {
            return cells.back();
        }

    private:

        /// <summary>
        /// Adds the entity to the specified cell.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="bounds"></param>
        /// <param name="cellIndex"></param>
        void addEntityTo(Entity entity, bounds::AABB bounds, uint32_t cellIndex) noexcept
        {
            entityToCell[entity.index] = cellIndex;
            entityToCellSlot[entity.index] = cells[cellIndex].entities.size();
            cells[cellIndex].entities.push_back(entity);
            cells[cellIndex].entityBounds.push_back(bounds);
        }

        /// <summary>
        /// Removes the entity from the specified cell.
        /// 
        /// This removal is performed via swap-and-pop and so the last entity in the cell
        /// is swapped into the slot occupied by the entity being removed.
        /// </summary>
        /// <param name="cellIndex"></param>
        /// <param name="cellSlot"></param>
        void removeEntityFrom(Entity entity, uint32_t cellIndex, uint32_t cellSlot) noexcept
        {
            auto& cell = cells[cellIndex];
            const auto lastSlot = cell.count() - 1;

            if (cellSlot != lastSlot)
            {
                const auto swappedEntity = cell.entities.back();
                const auto swappedAABB = cell.entityBounds.back();

                cell.entities[cellSlot] = swappedEntity;
                cell.entityBounds[cellSlot] = swappedAABB;
                entityToCell[swappedEntity.index] = cellIndex;
                entityToCellSlot[swappedEntity.index] = cellSlot;
            }

            cell.entities.pop_back();
            cell.entityBounds.pop_back();
            entityToCell.erase(entity.index);
            entityToCellSlot.erase(entity.index);
        }
    };

    UniformGridPartition::UniformGridPartition()
    {

    }

    UniformGridPartition::UniformGridPartition(UniformGridOptions const& options)
    {
        configure(options);
    }

    UniformGridPartition::~UniformGridPartition()
    {

    }

    void UniformGridPartition::configure(UniformGridOptions const& options) noexcept
    {
        LITL_FATAL_ASSERT_MSG(
            options.isValid(),
            "UniformGridPartition must have both .cellSize and .cellCount that are power-of-twos and greater than one.");

        m_impl->options = options;
        m_impl->cells.reserve(options.cellCount * options.cellCount);

        for (auto z = 0u; z < options.cellCount; ++z)
        {
            for (auto x = 0u; x < options.cellCount; ++x)
            {
                m_impl->cells.emplace_back(
                    options.origin.x() + static_cast<float>(x * options.cellSize),
                    options.origin.z() + static_cast<float>(z * options.cellSize),
                    static_cast<float>(options.cellSize),
                    options.yMin,
                    options.yMax);
            }
        }

        // add one extra cell at the back to contain oversized entities
        m_impl->cells.emplace_back(
            options.origin.x(),
            options.origin.z(),
            static_cast<float>(options.cellCount * options.cellSize),
            options.yMin,
            options.yMax
        );

        m_impl->oversizedEntityThreshold = static_cast<float>(options.cellSize * options.cellSize) * 0.5f;
    }

    void UniformGridPartition::add(Entity entity, bounds::AABB bounds) noexcept
    {
        m_impl->add(entity, bounds);
    }

    void UniformGridPartition::remove(Entity entity) noexcept
    {
        m_impl->remove(entity);
    }

    void UniformGridPartition::update(Entity entity, bounds::AABB bounds) noexcept
    {
        m_impl->update(entity, bounds);
    }

    void UniformGridPartition::query(bounds::AABB aabb, std::vector<Entity>& entities) const noexcept
    {
        m_impl->query(aabb, entities);
    }

    void UniformGridPartition::query(bounds::Sphere sphere, std::vector<Entity>& entities) const noexcept
    {
        m_impl->query(sphere, entities);
    }

    void UniformGridPartition::query(bounds::Frustum const& frustum, std::vector<Entity>& entities) const noexcept
    {
        m_impl->query(frustum, entities);
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

    uint32_t UniformGridPartition::getCellPopulation(uint32_t cellX, uint32_t cellZ) const noexcept
    {
        LITL_ASSERT_MSG((cellX < m_impl->options.cellCount) && (cellZ < m_impl->options.cellCount), "Requested cell for UniformGridPartition is out-of-bounds", 0);
        return m_impl->cells[m_impl->getIndex(cellX, cellZ)].count();
    }

    uint32_t UniformGridPartition::getOversizedCellPopulation() const noexcept
    {
        return m_impl->getOversizedCell().count();
    }

    uint32_t UniformGridPartition::getGridPopulation() const noexcept
    {
        return static_cast<uint32_t>(m_impl->entityToCell.size());
    }

    std::pair<uint32_t, uint32_t> UniformGridPartition::getCellIndex(vec3 worldPos) const noexcept
    {
        return { m_impl->getCellIndexX(worldPos.x()), m_impl->getCellIndexZ(worldPos.z()) };
    }

    std::optional<UniformGridEntityInfo> UniformGridPartition::getEntityInfo(EntityId entityId) const noexcept
    {
        const auto findCell = m_impl->entityToCell.find(entityId);

        if (findCell == m_impl->entityToCell.end())
        {
            return std::nullopt;
        }

        const auto cellIndex = findCell->second;
        const auto cellSlot = m_impl->entityToCellSlot.find(entityId)->second;

        return UniformGridEntityInfo{
            .entity = m_impl->cells[cellIndex].entities[cellSlot],
            .bounds = m_impl->cells[cellIndex].entityBounds[cellSlot],
            .cellIndex = cellIndex,
            .isOversized = m_impl->isOversized(m_impl->cells[cellIndex].entityBounds[cellSlot])
        };
    }
}