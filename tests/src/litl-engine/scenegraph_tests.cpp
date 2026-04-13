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

    LITL_TEST_CASE("track", "[scenegraph]")
    {
        SceneGraph scene;

        Entity entity{ .index = 0, .version = 0 };
        Transform transform{};

        REQUIRE(scene.size() == 0);
        REQUIRE(scene.isPresent(entity) == false);
        
        scene.track(entity, transform, TestSceneGraphAccessKey{});

        REQUIRE(scene.size() == 1);
        REQUIRE(scene.isPresent(entity) == true);

    } END_LITL_TEST_CASE
}