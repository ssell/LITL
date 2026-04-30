#include <algorithm>
#include "litl-engine/scene/sceneCommandProcessor.hpp"

namespace litl
{
    void SceneCommandProcessor::process(std::shared_ptr<Scene> scene, std::span<EntitySceneCommand const> sceneCommands) noexcept
    {
        if ((scene == nullptr) || sceneCommands.empty())
        {
            return;
        }
        
        sortCommands(sceneCommands);

        for (auto& sceneCommand : sceneCommands)
        {
            // Process the commands. They are ordered by entity and then command type (Destroy -> Create -> SetParent)
            // Any destroy will invalidate any following commands for that entity.

            // ... todo ...
            // ... also need to track destroyed entities to invalidate an entity having its parent set to it ...
            // ... also remember the underlying scene structures do not validate if an entity exists in it before operating on it ...
            // ... failure to maintain order/correctness here will results in asserts downstream ...
        }

        m_sortedCommands.clear();
    }

    void SceneCommandProcessor::sortCommands(std::span<EntitySceneCommand const> sceneCommands) noexcept
    {
        // Just like in the EntityCommandProcessor, we want to first sort the commands on: Entity -> command type
        m_sortedCommands.reserve(sceneCommands.size());
        m_sortedCommands.assign(sceneCommands.begin(), sceneCommands.end());

        std::sort(m_sortedCommands.begin(), m_sortedCommands.end(), [](EntitySceneCommand a, EntitySceneCommand b) -> bool
            {
                if (a.entity != b.entity)
                {
                    return (a.entity.index < b.entity.index);
                }
                else
                {
                    return static_cast<uint32_t>(a.type) < static_cast<uint32_t>(b.type);
                }
            });
    }
}