#ifndef LITL_ECS_ENTITY_SCENE_COMMAND_H__
#define LITL_ECS_ENTITY_SCENE_COMMAND_H__

namespace litl
{
    enum class EntitySceneCommandType : uint32_t
    {
        None = 0,
        CreateEntity = 1,
        DestroyEntity = 2,
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