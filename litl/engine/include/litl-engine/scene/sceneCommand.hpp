#ifndef LITL_ENGINE_SCENE_COMMAND_H__
#define LITL_ENGINE_SCENE_COMMAND_H__

#include "litl-ecs/entity/entity.hpp"

namespace litl
{
    enum class SceneCommandType : uint32_t
    {
        None = 0,
        RemoveFromGraph = 1,
        AddToGraph = 2,
        RemoveFromPartition = 3,
        AddToPartition = 4,
        SetParent = 5
    };

    struct SceneCommand
    {
        SceneCommandType type{ SceneCommandType::None };
        Entity entity{};
        Entity parent{};
    };
}

#endif