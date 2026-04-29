#ifndef LITL_ECS_FRAME_CALLBACKS_H__
#define LITL_ECS_FRAME_CALLBACKS_H__

#include <array>
#include <vector>

#include "litl-ecs/system/systemGroup.hpp"
#include "litl-ecs/entity/entitySceneCommand.hpp"

namespace litl
{
    using OnECSFrameStartFunc = void(*)();
    using OnECSPreGroupRunFunc = void(*)(SystemGroup group);
    using OnECSPostGroupRunFunc = void(*)(SystemGroup group);
    using OnECSSyncPointFunc = void(*)(SystemGroup group, std::vector<EntitySceneCommand> const& sceneCommands);
    using OnECSFrameEndFunc = void(*)();

    struct ECSFrameCallbacks
    {
        OnECSFrameStartFunc frameStartCallback { nullptr };
        OnECSSyncPointFunc syncPointCallback { nullptr };
        OnECSFrameEndFunc frameEndCallback { nullptr };

        std::array<OnECSPreGroupRunFunc, static_cast<uint32_t>(SystemGroup::Count)> preGroupCallbacks;
        std::array<OnECSPostGroupRunFunc, static_cast<uint32_t>(SystemGroup::Count)> postGroupCallbacks;
    };
}


#endif