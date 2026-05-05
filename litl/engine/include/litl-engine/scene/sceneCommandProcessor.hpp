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

        void process(Scene& scene, World& world, std::span<EntityChange const> entityChanges) noexcept;

    protected:

    private:

        void sortCommands(std::span<EntityChange const> entityChanges) noexcept;

        void onDestroyEntity(Scene& scene, World& world, EntityChange const& change) const noexcept;
        void onChangeArchetype(Scene& scene, World& world, EntityChange const& change) const noexcept;
        void onSetParent(Scene& scene, World& world, EntityChange const& change) const noexcept;

        /// <summary>
        /// The sorted Entity changes.
        /// Kept as a member (instead of just a local in the process method) to potentially avoid reallocations.
        /// </summary>
        std::vector<EntityChange> m_sortedChanges;
    };
}

#endif