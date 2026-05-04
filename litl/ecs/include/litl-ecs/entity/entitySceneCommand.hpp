#ifndef LITL_ECS_ENTITY_SCENE_COMMAND_H__
#define LITL_ECS_ENTITY_SCENE_COMMAND_H__

#include "litl-ecs/entity/entity.hpp"

namespace litl
{
    enum class EntitySceneCommandType : uint32_t
    {
        None = 0,
        UntrackEntity = 1,
        TrackEntity = 2,
        SetParent = 3
    };

    // Ordering is important as it is used in command buffer sorting
    static_assert(EntitySceneCommandType::UntrackEntity < EntitySceneCommandType::TrackEntity);

    /// <summary>
    /// An entity command that modifies the scene structure.
    /// </summary>
    struct EntitySceneCommand
    {
        EntitySceneCommandType type{ EntitySceneCommandType::None };
        Entity entity{};
        Entity parent{};
    };
}

#endif