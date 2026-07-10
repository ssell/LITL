#ifndef LITL_ECS_ENTITY_PARENT_ENTITY_H__
#define LITL_ECS_ENTITY_PARENT_ENTITY_H__

#include "litl-core/authority.hpp"
#include "litl-ecs/entity/entity.hpp"

namespace litl
{
    class SceneChangeProcessor;

    /// <summary>
    /// Wrapper around the parent entity to avoid accidental non-deferred parent writes.
    /// 
    /// Re-parenting is a structural change, it is not allowed to be done directly within
    /// a system, as systems are run over chunks in parallel, and ordering is not guaranteed.
    /// As such, the actual act of changing parents must be done via a deferred entity command
    /// at a sync point, so that the changes do not conflict with any work running in parallel.
    /// </summary>
    struct ParentEntity
    {
        ParentEntity()
        {

        }

        [[nodiscard]] constexpr Entity get() const noexcept
        {
            return value;
        }

        constexpr void set(Entity parent, Authority<SceneChangeProcessor> authority) noexcept
        {
            value = parent;
        }

        [[nodiscard]] static ParentEntity create(Entity parent, Authority<SceneChangeProcessor> authority) noexcept
        {
            return ParentEntity{ parent };
        }

        [[nodiscard]] constexpr bool has() const noexcept
        {
            return !value.isNull();
        }

    private:

        ParentEntity(Entity parent)
            : value(parent)
        {

        }

        Entity value{ Entity::null() };
    };
}

#endif