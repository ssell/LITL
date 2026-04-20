#include "litl-core/assert.hpp"
#include "litl-engine/scene/partition/uniformGridPartition.hpp"

namespace litl
{
    struct UniformGridPartition::Impl
    {

    };

    UniformGridPartition::UniformGridPartition(UniformGridOptions options)
        : m_options(options)
    {
        LITL_FATAL_ASSERT_MSG(
            options.isValid(), 
            "UniformGridPartition must have both .cellSize and .cellCount that are power-of-twos and greater than one.");
    }

    UniformGridPartition::~UniformGridPartition()
    {

    }

    void UniformGridPartition::add(EntityId entityId, bounds::AABB bounds) noexcept
    {

    }

    void UniformGridPartition::remove(EntityId entityId) noexcept
    {

    }

    void UniformGridPartition::update(EntityId entityId, bounds::AABB bounds) noexcept
    {

    }

    void UniformGridPartition::query(bounds::AABB aabb, std::vector<EntityId>& entities) const noexcept
    {

    }

    void UniformGridPartition::query(bounds::Sphere sphere, std::vector<EntityId>& entities) const noexcept
    {

    }

    void UniformGridPartition::query(bounds::Frustum const& frustum, std::vector<EntityId>& entities) const noexcept
    {

    }

    uint32_t UniformGridPartition::getCellSize() const noexcept
    {
        return m_options.cellSize;
    }

    uint32_t UniformGridPartition::getCellCount() const noexcept
    {
        return m_options.cellCount;
    }

    uint32_t UniformGridPartition::getWorldSize() const noexcept
    {
        return (m_options.cellSize * m_options.cellCount);
    }
}