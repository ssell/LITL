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
}