#include "tests.hpp"
#include "litl-engine/scene/sceneChangeProcessor.hpp"
#include "litl-ecs/archetype/archetypeRegistry.hpp"

namespace litl::tests
{
    namespace
    {
        Entity createEntity(World& world, std::vector<EntityChange>& changes)
        {
            Entity entity = world.createImmediate();

            changes.push_back(EntityChange{
                .type = EntityChangeType::CreateEntity,
                .entity = entity
            });

            return entity;
        }

        void destroyEntity(Entity entity, World& world, std::vector<EntityChange>& changes)
        {
            world.destroyImmediate(entity);

            changes.push_back(EntityChange{
                .type = EntityChangeType::DestroyEntity,
                .entity = entity
            });
        }

        void addTransform(Entity entity, World& world, std::vector<EntityChange>& changes)
        {
            auto prevArchetypeId = world.getEntityRecord(entity).archetype->id();
            world.addComponentImmediate<Transform>(entity);

            changes.push_back(EntityChange{
                .type = EntityChangeType::ChangeArchetype,
                .entity = entity,
                .prevArchetype = prevArchetypeId,
                .currArchetype = world.getEntityRecord(entity).archetype->id()
            });
        }

        void removeTransform(Entity entity, World& world, std::vector<EntityChange>& changes)
        {
            auto prevArchetypeId = world.getEntityRecord(entity).archetype->id();
            world.removeComponentImmediate<Transform>(entity);

            changes.push_back(EntityChange{
                .type = EntityChangeType::ChangeArchetype,
                .entity = entity,
                .prevArchetype = prevArchetypeId,
                .currArchetype = world.getEntityRecord(entity).archetype->id()
            });
        }

        Entity createTrackableEntity(World& world, std::vector<EntityChange>& changes)
        {
            Entity entity = createEntity(world, changes);
            addTransform(entity, world, changes);
            return entity;
        }

        void setParent(Entity child, Entity parent, std::vector<EntityChange>& changes)
        {
            changes.push_back(EntityChange{
                .type = EntityChangeType::SetParent,
                .entity = child,
                .parent = parent
            });
        }

        void processChanges(SceneChangeProcessor& processor, World& world, Scene& scene, std::vector<EntityChange>& changes)
        {
            processor.process(scene, world, changes);
            changes.clear();
        }
    }

    LITL_TEST_CASE("Basic Operations", "[engine::sceneChangeProcessor]")
    {
        World world{};
        Scene scene{ SceneConfig{} };
        SceneChangeProcessor processor{};
        std::vector<EntityChange> changes;

        // CreateEntity Change
        Entity entity = createEntity(world, changes);
        processChanges(processor, world, scene, changes);
        REQUIRE(scene.isPresent(entity) == false);      // no transform = not in scene

        // ChangeArchetype Change
        addTransform(entity, world, changes);
        processChanges(processor, world, scene, changes);
        REQUIRE(scene.isPresent(entity) == true);

        // DestroyEntity Change
        destroyEntity(entity, world, changes);
        processChanges(processor, world, scene, changes);
        REQUIRE(scene.isPresent(entity) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Remove Transform Untracks", "[engine::sceneChangeProcessor]")
    {
        World world{};
        Scene scene{ SceneConfig{} };
        SceneChangeProcessor processor{};
        std::vector<EntityChange> changes;

        Entity entity = createEntity(world, changes);
        addTransform(entity, world, changes);
        processChanges(processor, world, scene, changes);

        REQUIRE(scene.isPresent(entity) == true);

        removeTransform(entity, world, changes);
        processChanges(processor, world, scene, changes);

        REQUIRE(scene.isPresent(entity) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Cascade destroy", "[engine::sceneChangeProcessor]")
    {
        // Destroying an entity should also destroy all descendants.

        World world{};
        Scene scene{ SceneConfig{} };
        SceneChangeProcessor processor{};
        std::vector<EntityChange> changes;

        Entity parent = createTrackableEntity(world, changes);
        Entity child0 = createTrackableEntity(world, changes);
        Entity child1 = createTrackableEntity(world, changes);
        Entity gchild0 = createTrackableEntity(world, changes);
        Entity unrelated = createTrackableEntity(world, changes);

        setParent(child0, parent, changes);
        setParent(child1, parent, changes);
        setParent(gchild0, child0, changes);
        processChanges(processor, world, scene, changes);

        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.isPresent(child0) == true);
        REQUIRE(scene.isPresent(child1) == true);
        REQUIRE(scene.isPresent(gchild0) == true);
        REQUIRE(scene.isPresent(unrelated) == true);

        destroyEntity(parent, world, changes);
        processChanges(processor, world, scene, changes);

        REQUIRE(scene.isPresent(parent) == false);
        REQUIRE(scene.isPresent(child0) == false);
        REQUIRE(scene.isPresent(child1) == false);
        REQUIRE(scene.isPresent(gchild0) == false);
        REQUIRE(scene.isPresent(unrelated) == true);

        REQUIRE(world.isAlive(parent) == false);
        REQUIRE(world.isAlive(child0) == false);
        REQUIRE(world.isAlive(child1) == false);
        REQUIRE(world.isAlive(gchild0) == false);
        REQUIRE(world.isAlive(unrelated) == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Destroy child", "[engine::sceneChangeProcessor]")
    {
        // Destroying a child should not destroy other children or the parent.

        World world{};
        Scene scene{ SceneConfig{} };
        SceneChangeProcessor processor{};
        std::vector<EntityChange> changes;

        Entity parent = createTrackableEntity(world, changes);
        Entity child0 = createTrackableEntity(world, changes);
        Entity child1 = createTrackableEntity(world, changes);
        Entity gchild0 = createTrackableEntity(world, changes);
        Entity unrelated = createTrackableEntity(world, changes);

        setParent(child0, parent, changes);
        setParent(child1, parent, changes);
        setParent(gchild0, child0, changes);
        processChanges(processor, world, scene, changes);

        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.isPresent(child0) == true);
        REQUIRE(scene.isPresent(child1) == true);
        REQUIRE(scene.isPresent(gchild0) == true);
        REQUIRE(scene.isPresent(unrelated) == true);

        // Destroy child0 which should also destroy gchild0
        destroyEntity(child0, world, changes);
        processChanges(processor, world, scene, changes);

        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.isPresent(child0) == false);
        REQUIRE(scene.isPresent(child1) == true);
        REQUIRE(scene.isPresent(gchild0) == false);
        REQUIRE(scene.isPresent(unrelated) == true);

        REQUIRE(world.isAlive(parent) == true);
        REQUIRE(world.isAlive(child0) == false);
        REQUIRE(world.isAlive(child1) == true);
        REQUIRE(world.isAlive(gchild0) == false);
        REQUIRE(world.isAlive(unrelated) == true);
    } LITL_END_TEST_CASE

        LITL_TEST_CASE("Reparent Prevents Destruction", "[engine::sceneChangeProcessor]")
    {
        // If gchild0 is reparented to child1 at the same time child0 is destroyed, it should _not_ be destroyed.

        World world{};
        Scene scene{ SceneConfig{} };
        SceneChangeProcessor processor{};
        std::vector<EntityChange> changes;

        Entity parent = createTrackableEntity(world, changes);
        Entity child0 = createTrackableEntity(world, changes);
        Entity child1 = createTrackableEntity(world, changes);
        Entity gchild0 = createTrackableEntity(world, changes);
        Entity unrelated = createTrackableEntity(world, changes);

        setParent(child0, parent, changes);
        setParent(child1, parent, changes);
        setParent(gchild0, child0, changes);
        processChanges(processor, world, scene, changes);

        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.isPresent(child0) == true);
        REQUIRE(scene.isPresent(child1) == true);
        REQUIRE(scene.isPresent(gchild0) == true);
        REQUIRE(scene.isPresent(unrelated) == true);

        // Destroy child0 but also reparent gchild0 to child1 to save it.
        destroyEntity(child0, world, changes);
        setParent(gchild0, child1, changes);
        processChanges(processor, world, scene, changes);

        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.isPresent(child0) == false);
        REQUIRE(scene.isPresent(child1) == true);
        REQUIRE(scene.isPresent(gchild0) == true);
        REQUIRE(scene.isPresent(unrelated) == true);

        REQUIRE(world.isAlive(parent) == true);
        REQUIRE(world.isAlive(child0) == false);
        REQUIRE(world.isAlive(child1) == true);
        REQUIRE(world.isAlive(gchild0) == true);
        REQUIRE(world.isAlive(unrelated) == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Dedupe Destruction", "[engine::sceneChangeProcessor]")
    {
        // Explicitly calling destroy on a child whose parent is being destroyed should be deduped.

        World world{};
        Scene scene{ SceneConfig{} };
        SceneChangeProcessor processor{};
        std::vector<EntityChange> changes;

        Entity parent = createTrackableEntity(world, changes);
        Entity child0 = createTrackableEntity(world, changes);
        Entity child1 = createTrackableEntity(world, changes);
        Entity gchild0 = createTrackableEntity(world, changes);
        Entity unrelated = createTrackableEntity(world, changes);

        setParent(child0, parent, changes);
        setParent(child1, parent, changes);
        setParent(gchild0, child0, changes);
        processChanges(processor, world, scene, changes);

        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.isPresent(child0) == true);
        REQUIRE(scene.isPresent(child1) == true);
        REQUIRE(scene.isPresent(gchild0) == true);
        REQUIRE(scene.isPresent(unrelated) == true);

        destroyEntity(parent, world, changes);              // only one that matters, the rest will be deduped
        destroyEntity(child0, world, changes);
        destroyEntity(child1, world, changes);
        destroyEntity(gchild0, world, changes);
        processChanges(processor, world, scene, changes);

        REQUIRE(scene.isPresent(parent) == false);
        REQUIRE(scene.isPresent(child0) == false);
        REQUIRE(scene.isPresent(child1) == false);
        REQUIRE(scene.isPresent(gchild0) == false);
        REQUIRE(scene.isPresent(unrelated) == true);

        REQUIRE(world.isAlive(parent) == false);
        REQUIRE(world.isAlive(child0) == false);
        REQUIRE(world.isAlive(child1) == false);
        REQUIRE(world.isAlive(gchild0) == false);
        REQUIRE(world.isAlive(unrelated) == true);
    } LITL_END_TEST_CASE
}