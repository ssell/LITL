#ifndef LITL_ENGINE_SCENE_COMMAND_PROCESSOR_H__
#define LITL_ENGINE_SCENE_COMMAND_PROCESSOR_H__

#include <memory>
#include <span>
#include <vector>

#include "litl-ecs/entity/entitySceneCommand.hpp"
#include "litl-engine/scene/scene.hpp"

namespace litl
{
    /// <summary>
    /// Processes a list of EntitySceneCommands output by the SceneManager into a specified Scene.
    /// </summary>
    class SceneCommandProcessor
    {
    public:

        void process(std::shared_ptr<Scene> scene, std::span<EntitySceneCommand const> sceneCommands) noexcept;

    protected:

    private:

        void sortCommands(std::span<EntitySceneCommand const> sceneCommands) noexcept;

        std::vector<EntitySceneCommand> m_allCommands;
    };
}

#endif