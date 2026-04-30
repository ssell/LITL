#ifndef LITL_ECS_ENTITY_SCENE_COMMAND_H__
#define LITL_ECS_ENTITY_SCENE_COMMAND_H__

#include "litl-ecs/entity/entity.hpp"

namespace litl
{
    enum class EntitySceneCommandType : uint32_t
    {
        None = 0,
        DestroyEntity = 1,
        CreateEntity = 2,
        SetParent = 3
    };

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