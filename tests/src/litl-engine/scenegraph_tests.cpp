#include "tests.hpp"
#include "litl-engine/scene/scenegraph.hpp"

namespace litl::tests
{
    namespace
    {
        struct TestSceneGraphAccessKey : SceneGraphAccessKey
        {
            TestSceneGraphAccessKey() {}
        };
    }

    LITL_TEST_CASE("track and untrack", "[scenegraph]")
    {
        SceneGraph scene;

        Entity entity{ .index = 0, .version = 0 };
        Transform transform{};

        REQUIRE(scene.size() == 0);
        REQUIRE(scene.isPresent(entity) == false);
        
        scene.track(entity, transform, TestSceneGraphAccessKey{});

        REQUIRE(scene.size() == 1);
        REQUIRE(scene.isPresent(entity) == true);

        // adding a second time should throw an assert
        LITL_START_ASSERT_CAPTURE
            scene.track(entity, transform, TestSceneGraphAccessKey{});
        LITL_END_ASSERT_CAPTURE

        scene.untrackEntity(entity, SceneGraphUntrackBehavior::Cascade, TestSceneGraphAccessKey{});

        REQUIRE(scene.size() == 0);
        REQUIRE(scene.isPresent(entity) == false);
    } END_LITL_TEST_CASE
}