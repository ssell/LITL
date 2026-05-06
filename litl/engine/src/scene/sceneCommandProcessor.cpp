#include <algorithm>

#include "litl-ecs/archetype/archetypeRegistry.hpp"
#include "litl-engine/scene/sceneCommandProcessor.hpp"
#include "litl-engine/ecs/components/transform.hpp"
#include "litl-engine/ecs/components/bounds.hpp"

namespace litl
{
    void SceneCommandProcessor::process(Scene& scene, World& world, std::span<EntityChange const> entityChanges) noexcept
    {
        if (entityChanges.size() == 0)
        {
            return;
        }

        sortCommands(entityChanges);

        // The logic below is dependent on the EntityCommandProcessor performing deduplication and remove non-SetParent commands
        // when an entity is also being destroyed at the same time. This allows the logic below to avoid having to perform checks itself.

        for (auto& change : m_sortedChanges)
        {
            switch (change.type)
            {
            case EntityChangeType::CreateEntity:
                // Create on its own does not impact the scene. A newly created entity has no components and thus can not be tracked.
                break;

            case EntityChangeType::DestroyEntity:
                onDestroyEntity(scene, world, change);
                break;

            case EntityChangeType::ChangeArchetype:
                onChangeArchetype(scene, world, change);
                break;

            case EntityChangeType::SetParent:
                onSetParent(scene, world, change);
                break;

            default:
                break;
            }
        }
    }

    void SceneCommandProcessor::sortCommands(std::span<EntityChange const> entityChanges) noexcept
    {
        m_sortedChanges.clear();
        m_sortedChanges.reserve(entityChanges.size());
        m_sortedChanges.assign(entityChanges.begin(), entityChanges.end());

        std::ranges::stable_sort(m_sortedChanges, [](EntityChange a, EntityChange b)
        {
            // SetParent commands go last, everything else groups by entity
            const bool aIsSetParent = a.type == EntityChangeType::SetParent;
            const bool bIsSetParent = b.type == EntityChangeType::SetParent;

            // Create a tuple of references. Tuple comparison is lexicographic, so it compares SetParent first then entity and then command type.
            return std::tie(aIsSetParent, a.entity, a.type) < std::tie(bIsSetParent, b.entity, b.type);
        });
    }

    void SceneCommandProcessor::onDestroyEntity(Scene& scene, World& world, EntityChange const& change) const noexcept
    {
        auto* archetype = ArchetypeRegistry::getById(change.prevArchetype);

        if (archetype->hasComponent<Transform>())
        {
            scene.untrack(change.entity);
        }
    }

    void SceneCommandProcessor::onChangeArchetype(Scene& scene, World& world, EntityChange const& change) const noexcept
    {
        if (change.prevArchetype == change.currArchetype)
        {
            return;
        }

        auto* prevArchetype = ArchetypeRegistry::getById(change.prevArchetype);
        auto* currArchetype = ArchetypeRegistry::getById(change.currArchetype);

        bool prevHadTransform = prevArchetype->hasComponent<Transform>();
        bool prevHadBounds = prevArchetype->hasComponent<WorldBounds>();

        bool currHasTransform = currArchetype->hasComponent<Transform>();
        bool currHasBounds = currArchetype->hasComponent<WorldBounds>();

        if (prevHadTransform && !currHasTransform)
        {
            // Lost the Transform component. Can no longer be tracked.
            scene.untrack(change.entity);
        }
        else
        {
            auto transform = currArchetype->getComponent<Transform>(world.getEntityRecord(change.entity));

            if (prevHadBounds && !currHasBounds)
            {
                // Lost the Bounds component. Update the scene partition to use a unit cube bounds.
                scene.update(change.entity, bounds::AABB::fromCenterHalfExtents(transform.getPosition(), vec3{0.5f, 0.5f, 0.5f}));
            }
            else if (!prevHadTransform && currHasTransform)
            {
                // Gained the Transform component. Start tracking it.
                // At this point dont worry about the bounds. That will be updated in the WorldBoundsSystem if it is present.
                scene.track(change.entity, transform);
            }
        }
    }

    void SceneCommandProcessor::onSetParent(Scene& scene, World& world, EntityChange const& change) const noexcept
    {
        scene.setParent(change.entity, change.parent);
    }
}