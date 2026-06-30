#include <algorithm>

#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-ecs/archetype/archetypeRegistry.hpp"
#include "litl-engine/scene/sceneChangeProcessor.hpp"
#include "litl-engine/ecs/components/transform.hpp"
#include "litl-engine/ecs/components/bounds.hpp"

namespace litl
{
    void SceneChangeProcessor::process(Scene& scene, World& world, std::span<EntityChange const> entityChanges) noexcept
    {
        if (entityChanges.size() == 0)
        {
            return;
        }

        /**
         * The logic below is dependent on the EntityCommandProcessor performing deduplication and remove non-SetParent commands
         * when an entity is also being destroyed at the same time. This allows the logic below to avoid having to perform checks itself.
         * 
         * We perform Create -> Change Archetype -> Set Parent first on all entities.
         * 
         * And then we find all entities that will be destroyed. This includes both those being explicity destroyed in an EntityChange
         * and also their descendants. 
         */

        sortCommands(entityChanges);

        size_t i = 0ull;

        for (; i < m_sortedChanges.size(); ++i)
        {
            auto& change = m_sortedChanges[i];

            LITL_ASSERT_MSG((!change.entity.isNull()), "SceneChangeProcessor provided with a NULL entity", );

            if (change.type == EntityChangeType::DestroyEntity)
            {
                // All destroys are placed at the end of the list (see sortCommands).
                // We need to gather descendants before we can start processing destroys.
                break;
            }

            switch (change.type)
            {
            case EntityChangeType::ChangeArchetype:
                onChangeArchetype(scene, world, change);
                break;

            case EntityChangeType::SetParent:
                onSetParent(scene, world, change);
                break;

            case EntityChangeType::CreateEntity:            // Create on its own does not impact the scene. A newly created entity has no components and thus can not be tracked.
            default:
                break;
            }
        }

        if (i != m_sortedChanges.size())
        {
            // There are destroys to process. Gather all affected entities - those being destroyed AND their descendants.
            m_doomedEntities.clear();
            m_doomedEntities.reserve(m_sortedChanges.size() - i);

            for (; i < m_sortedChanges.size(); ++i)
            {
                auto& destroyChange = m_sortedChanges[i];

                if (destroyChange.type != EntityChangeType::DestroyEntity)
                {
                    logWarning("Unexpected EntityChangeType in destroy path of SceneChangeProcessor::process");
                    continue;
                }

                if (scene.isPresent(destroyChange.entity))
                {
                    m_doomedEntities.push_back(destroyChange.entity);
                    scene.getChildren(destroyChange.entity, m_doomedEntities, true);
                }
            }
            
            // Dedupe as child entities could have been both destroyed explicitly by the user and added transitively by the above operation.
            m_dedupedDoomedEntities.clear();

            for (auto doomedEntity : m_doomedEntities)
            {
                m_dedupedDoomedEntities.insert(doomedEntity);
            }

            // Remove all doomed entities from the scene and the ECS.
            // Note that the original parent entities are already destroyed, but for simplicity we don't discern from them here.
            // Their internal ID state is already marked invalid and so the ECS will skip over them. A little wasteful but makes this logic cleaner.
            for (auto doomedEntity : m_dedupedDoomedEntities)
            {
                scene.untrack(doomedEntity);
                world.destroyImmediate(doomedEntity);       // Safe to call immediate here because we are at a sync point already (or should be!)
            }
        }

        scene.sync();
    }

    void SceneChangeProcessor::sortCommands(std::span<EntityChange const> entityChanges) noexcept
    {
        /**
         * Sorts the commands to be processed by the Scene.
         * Note that process order in the Scene differs from that in the ECS library.
         * For ECS we process commands in their enum order:
         * 
         *     Destroy -> Create -> Change Archetype -> Set Parent
         * 
         * By putting Destroy first in the ECS it lets it remove unnecessary commands
         * related to newly destroyed entities.
         * 
         * But for the Scene we want to process Destroy last so that an accurate collection
         * of affected descendent entities (so all children, grandchildren, etc. of a to-be
         * destroy entity) can be made. The ECS does not, and can not, do this because the
         * entity relationship hierarchy does not exist in the ECS library and exists within
         * the Scene. So for the Scene we prioritize changes in the order of:
         * 
         *    Create -> Change Archetype -> Set Parent -> Destroy
         * 
         * This cleans up the logic relating to destruction, especially when an affected
         * entity has its parentage changed in the same change set.
         */

        m_sortedChanges.clear();
        m_sortedChanges.reserve(entityChanges.size());
        m_sortedChanges.assign(entityChanges.begin(), entityChanges.end());

        std::ranges::stable_sort(m_sortedChanges, [](EntityChange a, EntityChange b)
        {
            // DestroyEntity commands go last, everything else groups by entity
            const bool aIsDestroy = a.type == EntityChangeType::DestroyEntity;
            const bool bIsDestroy = b.type == EntityChangeType::DestroyEntity;

            // Create a tuple of references. Tuple comparison is lexicographic, so it compares DestroyEntity first then entity and then command type.
            return std::tie(aIsDestroy, a.entity, a.type) < std::tie(bIsDestroy, b.entity, b.type);
        });
    }

    void SceneChangeProcessor::onChangeArchetype(Scene& scene, World& world, EntityChange const& change) const noexcept
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
        else if (currHasTransform)
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

    void SceneChangeProcessor::onSetParent(Scene& scene, World& world, EntityChange const& change) const noexcept
    {
        scene.setParent(change.entity, change.parent);
    }
}