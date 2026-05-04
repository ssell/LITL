#ifndef LITL_ENGINE_SCENE_COMMAND_PROCESSOR_H__
#define LITL_ENGINE_SCENE_COMMAND_PROCESSOR_H__

#include <memory>
#include <span>
#include <vector>

#include "litl-engine/scene/scene.hpp"
#include "litl-ecs/entity/entitySceneCommand.hpp"
#include "litl-ecs/world.hpp"

namespace litl
{
    /// <summary>
    /// Processes a list of EntitySceneCommands output by the SceneManager into a specified Scene.
    /// </summary>
    class SceneCommandProcessor
    {
    public:

        void process(Scene& scene, World& world, std::span<EntitySceneCommand const> sceneCommands) noexcept;

    protected:

    private:

        void sortCommands(std::span<EntitySceneCommand const> sceneCommands) noexcept;

        /// <summary>
        /// The sorted scene commands.
        /// Kept as a member (instead of just a local in the process method) to potentially avoid reallocations.
        /// </summary>
        std::vector<EntitySceneCommand> m_allCommands;
    };
}

#endif