#include "tests.hpp"
#include "litl-engine/scene/sceneChangeProcessor.hpp"
#include "litl-ecs/archetype/archetypeRegistry.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("Basic Operations", "[engine::sceneChangeProcessor]")
    {
        World world{};
        Scene scene{ SceneConfig{} };
        SceneChangeProcessor processor{};

        Entity entity = world.createImmediate();

        {
            EntityChange changes[]{
                EntityChange{
                    .type = EntityChangeType::CreateEntity,
                    .entity = entity
                }
            };

            processor.process(scene, world, changes);
        }
        
        REQUIRE(scene.isPresent(entity) == false);      // no transform = not in scene

        world.addComponentImmediate<Transform>(entity);

        {
            EntityChange changes[]{
                EntityChange {
                    .type = EntityChangeType::ChangeArchetype,
                    .entity = entity,
                    .prevArchetype = ArchetypeRegistry::Empty()->id(),
                    .currArchetype = ArchetypeRegistry::get<Transform>()->id()
                }
            };

            processor.process(scene, world, changes);
        }

        REQUIRE(scene.isPresent(entity) == true);       // gained transform = added to scene

    } LITL_END_TEST_CASE
}