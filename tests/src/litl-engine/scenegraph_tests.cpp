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

        struct TestParentEntityWriteKey : ParentEntityWriteKey
        {
            TestParentEntityWriteKey() {}
        };

        static TestSceneGraphAccessKey SceneGraphKey{};
        static TestParentEntityWriteKey ParentWriteKey{};
    }

    LITL_TEST_CASE("track and untrack", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity entity{ .index = 0, .version = 0 };
        Transform transform{};

        REQUIRE(scene.size() == 0);
        REQUIRE(scene.isPresent(entity) == false);
        
        scene.track(entity, transform, SceneGraphKey);

        REQUIRE(scene.size() == 1);
        REQUIRE(scene.isPresent(entity) == true);

        // adding a second time should throw an assert
        LITL_START_ASSERT_CAPTURE
            scene.track(entity, transform, SceneGraphKey);
        LITL_END_ASSERT_CAPTURE

        scene.untrackEntity(entity, SceneGraphUntrackBehavior::Cascade, SceneGraphKey);

        REQUIRE(scene.size() == 0);
        REQUIRE(scene.isPresent(entity) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("track with parent", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity child{ .index = 0, .version = 0 };
        Entity parent{ .index = 1, .version = 0 };

        Transform childTransform{};
        Transform parentTransform{};

        childTransform.parent.set(parent, ParentWriteKey);

        // fail to track child with untracked parent
        LITL_START_ASSERT_CAPTURE
            scene.track(child, childTransform, SceneGraphKey);
        LITL_END_ASSERT_CAPTURE

        childTransform.parent.set(child, ParentWriteKey);

        // fail to track child with self parentage
        LITL_START_ASSERT_CAPTURE
            scene.track(child, childTransform, SceneGraphKey);
        LITL_END_ASSERT_CAPTURE

        childTransform.parent.set(parent, ParentWriteKey);

        scene.track(parent, parentTransform, SceneGraphKey);
        scene.track(child, childTransform, SceneGraphKey);

        REQUIRE(scene.size() == 2);
        REQUIRE(scene.isPresent(child) == true);
        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.getParent(child) == parent);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("set parent", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity child{ .index = 0, .version = 0 };
        Entity parent{ .index = 1, .version = 0 };

        scene.track(child, Transform{}, SceneGraphKey);
        scene.track(parent, Transform{}, SceneGraphKey);

        REQUIRE(scene.size() == 2);
        REQUIRE(scene.isPresent(child) == true);
        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.getParent(child).isNull());

        LITL_START_ASSERT_CAPTURE
            scene.setParent(child, child, SceneGraphKey);
        LITL_END_ASSERT_CAPTURE

        scene.setParent(child, parent, SceneGraphKey);

        REQUIRE(scene.size() == 2);
        REQUIRE(scene.isPresent(child) == true);
        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.getParent(child) == parent);

        std::vector<Entity> children = scene.getChildren(parent);

        REQUIRE(children.size() == 1);
        REQUIRE(children[0] == child);

        scene.setParent(child, Entity::null(), SceneGraphKey);
        children = scene.getChildren(parent);

        REQUIRE(scene.size() == 2);
        REQUIRE(scene.isPresent(child));
        REQUIRE(scene.isPresent(parent));
        REQUIRE(scene.getParent(child).isNull());
        REQUIRE(children.size() == 0);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("untrack", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity child{ .index = 0, .version = 0 };
        Entity parent{ .index = 1, .version = 0 };

        scene.track(child, Transform{}, SceneGraphKey);
        scene.track(parent, Transform{}, SceneGraphKey);
        scene.setParent(child, parent, SceneGraphKey);

        REQUIRE(scene.size() == 2);
        REQUIRE(scene.isPresent(child) == true);
        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.getParent(child) == parent);

        scene.untrackEntity(child, SceneGraphUntrackBehavior::Cascade, SceneGraphKey);

        REQUIRE(scene.size() == 1);
        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.isPresent(child) == false);

        LITL_START_ASSERT_CAPTURE
            REQUIRE(scene.getParent(child) == Entity::null());
        LITL_END_ASSERT_CAPTURE

        // cascade
        scene.track(child, Transform{}, SceneGraphKey);
        scene.setParent(child, parent, SceneGraphKey);

        REQUIRE(scene.size() == 2);
        REQUIRE(scene.isPresent(child) == true);
        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.getParent(child) == parent);

        scene.untrackEntity(parent, SceneGraphUntrackBehavior::Cascade, SceneGraphKey);
        
        REQUIRE(scene.size() == 0);
        REQUIRE(scene.isPresent(child) == false);
        REQUIRE(scene.isPresent(parent) == false);

        // orphan
        scene.track(parent, Transform{}, SceneGraphKey);
        scene.track(child, Transform{}, SceneGraphKey);
        scene.setParent(child, parent, SceneGraphKey);

        REQUIRE(scene.size() == 2);
        REQUIRE(scene.isPresent(child) == true);
        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.getParent(child) == parent);

        scene.untrackEntity(parent, SceneGraphUntrackBehavior::Orphan, SceneGraphKey);

        REQUIRE(scene.size() == 1);
        REQUIRE(scene.isPresent(child) == true);
        REQUIRE(scene.isPresent(parent) == false);
        REQUIRE(scene.getParent(child) == Entity::null());

    } LITL_END_TEST_CASE
}