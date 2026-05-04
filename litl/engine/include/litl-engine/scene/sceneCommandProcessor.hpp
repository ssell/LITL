#ifndef LITL_ENGINE_SCENE_COMMAND_PROCESSOR_H__
#define LITL_ENGINE_SCENE_COMMAND_PROCESSOR_H__

#include <memory>
#include <span>
#include <vector>

#include "litl-engine/scene/scene.hpp"
#include "litl-engine/scene/sceneCommand.hpp"
#include "litl-ecs/entity/entityCommand.hpp"
#include "litl-ecs/world.hpp"

namespace litl
{
    /// <summary>
    /// Processes a list of EntitySceneCommands output by the SceneManager into a specified Scene.
    /// </summary>
    class SceneCommandProcessor
    {
    public:

        void process(Scene& scene, World& world, std::span<EntityCommand const> entityCommands) noexcept;

    protected:

    private:

        void compileSceneCommands(std::span<EntityCommand const> entityCommands) noexcept;
        void sortCommands() noexcept;

        /// <summary>
        /// The sorted scene commands.
        /// Kept as a member (instead of just a local in the process method) to potentially avoid reallocations.
        /// </summary>
        std::vector<SceneCommand> m_allCommands;
    };
}

#endif