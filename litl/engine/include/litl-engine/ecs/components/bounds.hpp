#ifndef LITL_ENGINE_ECS_COMPONENTS_BOUNDS_H__
#define LITL_ENGINE_ECS_COMPONENTS_BOUNDS_H__

#include "litl-core/math/bounds.hpp"
#include "litl-ecs/constants.hpp"

namespace litl
{
    /// <summary>
    /// AABB in local-space. Typically only written to once (at entity creation/asset load/etc.).
    /// </summary>
    struct LocalBounds
    {
        bounds::AABB bounds{};
    };

    /// <summary>
    /// AABB in world-space. Updated automatically if the entity also has a Transform component.
    /// </summary>
    struct WorldBounds
    {
        bounds::AABB bounds{};

        /// <summary>
        /// The current version of the WorldBounds.
        /// Used to keep the bounds in sync with the Transform component.
        /// </summary>
        uint32_t version{ Constants::uint32_null_index };
    };

    static_assert(ValidComponentType<litl::LocalBounds>);
    static_assert(ValidComponentType<litl::WorldBounds>);
}

REGISTER_TYPE_NAME(litl::LocalBounds);
REGISTER_TYPE_NAME(litl::WorldBounds);

#endif