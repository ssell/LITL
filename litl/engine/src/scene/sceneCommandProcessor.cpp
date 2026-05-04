#include <algorithm>

#include "litl-engine/scene/sceneCommandProcessor.hpp"
#include "litl-engine/ecs/components/transform.hpp"
#include "litl-engine/ecs/components/bounds.hpp"

namespace litl
{

    void SceneCommandProcessor::process(Scene& scene, World& world, std::span<EntityCommand const> entityCommands) noexcept
    {
        compileSceneCommands(entityCommands);
    }

    void SceneCommandProcessor::compileSceneCommands(std::span<EntityCommand const> entityCommands) noexcept
    {
        m_allCommands.clear();

        // OK where am I at with all of this.
        // The EntityCommandProcessor needs to pass in clean, valid information. And then this is EZPZ.
        // CreateEntity -> no problem, and the scene does not care (as an entity creation does not involve components)
        // DestroyEntity -> need to know the archetype that the entity is leaving (did it have transform/bounds?)
        // AddComponent -> need to know the diff between its prev archetype and its new archetype. Is it gaining transform/bounds?
        // RemoveComponent -> need to know the diff between its prev archetype and its new archetype. Is it losing transform/bounds?
        // SetParent -> nothing else needed. if the user sets parent to an invalid entity (one without a transform) then that is their problemo
        // Furthermore, Scene needs to be updated in some way. Graph needs a transform, Partition needs a Transform+Bounds.
        // An entity can exist in the graph or graph+partition (but never just the partition).
        // Or well, maybe I dont agree with Claude on that one. It argues that some entities such as waypoints, audio emitters, etc.
        // dont need a bounds (sure) and that they dont need to appear in frustum culling (sure) or spatial queries (not so sure).
        // You could easily have a "spawn point" entity that doesnt have a bounds but you may have game logic that wants to find
        // all spawn points within a given radius or something. Is it wasteful having a bounds on it? Yes. But do 99% of entities
        // that have a transform also have a bounds? Yes.
        // So either all entities need a Transform+Bounds to be tracked in the Scene or the Spatial Partition needs to have bounds
        // optional and support entities that only have a transform. Hmmmm .....

        for (auto& entityCommand : entityCommands)
        {
            switch (entityCommand.type)
            {
            case EntityCommandType::CreateEntity:
                break;

            case EntityCommandType::DestroyEntity:
                break;

            case EntityCommandType::AddComponent:
                break;

            case EntityCommandType::RemoveComponent:
                break;

            case EntityCommandType::SetParent:
                m_allCommands.emplace_back(SceneCommandType::SetParent, entityCommand.entity, entityCommand.setParentInfo.parent);
                break;
            }
        }
    }

    void SceneCommandProcessor::sortCommands() noexcept
    {

    }

    /*
        if (sceneCommands.empty())
        {
            return;
        }
        
        sortCommands(sceneCommands);

        // Find the partition point between SetParent and the rest of the commands
        const auto firstSetParentIter = std::ranges::partition_point(m_allCommands, [](const EntitySceneCommand& cmd) { return cmd.type != EntitySceneCommandType::SetParent; });
        auto parentingCommands = std::ranges::subrange(firstSetParentIter, m_allCommands.end());

        for (auto commandIter = m_allCommands.begin(); commandIter != m_allCommands.end(); ++commandIter)
        {
            auto nextIter = commandIter + 1;
            auto& command = (*commandIter);

            // Process the commands. They are ordered by entity and then command type (Destroy -> Create -> SetParent)
            // Any destroy will invalidate any following commands for that entity.
            switch (command.type)
            {
            case EntitySceneCommandType::UntrackEntity:
            {
                // First check if next command is a CreateEntity. If so, the two commands cancel each other out. Very unlikely, but possible.
                if ((nextIter != m_allCommands.end()) &&
                    ((*nextIter).entity == command.entity) &&
                    ((*nextIter).type == EntitySceneCommandType::TrackEntity))
                {
                    command.type = EntitySceneCommandType::None;
                    (*nextIter).type = EntitySceneCommandType::None;
                }
                else
                {
                    // Invalidate all setParent commands that reference the soon to be destroy entity.
                    for (auto setParentIter = parentingCommands.begin(); setParentIter != parentingCommands.end(); ++setParentIter)
                    {
                        auto& setParentCommand = (*setParentIter);

                        if ((setParentCommand.entity == command.entity) || (setParentCommand.parent == command.entity))
                        {
                            setParentCommand.type = EntitySceneCommandType::None;
                        }
                    }

                    // Perform the removal
                    scene.remove(command.entity);
                }
                break;
            }

            case EntitySceneCommandType::TrackEntity:
            {
                auto transform = world.getComponent<Transform>(command.entity);
                auto bounds = world.getComponent<Bounds>(command.entity);

                LITL_ASSERT_MSG(transform.has_value() && bounds.has_value(), "Attempt to add Entity to scene that does not have both a Transform and Bounds component", );

                scene.add(command.entity, (*transform), (*bounds).bounds);
                break;
            }

            case EntitySceneCommandType::SetParent:
            {
                auto transform = world.getComponent<Transform>(command.entity);

                LITL_ASSERT_MSG(transform.has_value(), "Attempting update parent of entity missing a Transform component", );

                world.setComponent<Transform>(command.entity, Transform{
                    (*transform).rotation,
                    (*transform).position,
                    (*transform).uniformScale,
                    ParentEntity::create(command.parent, ParentEntityWriteKey{})
                });

                scene.setParent(command.entity, command.parent);
                break;
            }

            case EntitySceneCommandType::None:
            default:
                break;
            }
        }

        m_allCommands.clear();
    }

    void SceneCommandProcessor::sortCommands(std::span<EntityCommand const> sceneCommands) noexcept
    {
        // Sort all commands such that commands for the same entity are grouped together, except for SetParent which are placed at the end.
        // SetParent needs to be at the end as all of the track/untrack commands must be complete to ensure both parts (child and parent) of SetParent are valid.

        m_allCommands.reserve(sceneCommands.size());
        m_allCommands.assign(sceneCommands.begin(), sceneCommands.end());

        std::ranges::stable_sort(m_allCommands, [](EntityCommand a, EntityCommand b)
        {
            // SetParent commands go last, everything else groups by entity
            const bool aIsSetParent = a.type == EntityCommandType::SetParent;
            const bool bIsSetParent = b.type == EntityCommandType::SetParent;

            // Create a tuple of references. Tuple comparison is lexicographic, so it compares setParent first then entity and then command type.
            return std::tie(aIsSetParent, a.entity, a.type) < std::tie(bIsSetParent, b.entity, b.type);
        });
    }
    */
}