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

    LITL_TEST_CASE("rebuild produces valid topological order", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity a{ .index = 0, .version = 0 };  // root
        Entity b{ .index = 1, .version = 0 };  // child of a
        Entity c{ .index = 2, .version = 0 };  // child of b (grandchild of a)

        scene.track(a, Transform{}, SceneGraphKey);
        scene.track(b, Transform{}, SceneGraphKey);
        scene.track(c, Transform{}, SceneGraphKey);

        scene.setParent(b, a, SceneGraphKey);
        scene.setParent(c, b, SceneGraphKey);

        scene.update(SceneGraphKey);

        // Tree should have all 3 nodes
        REQUIRE(scene.size() == 3);

        // Verify parent-before-child invariant through getParent
        // After update, depths and sorted order should be consistent
        REQUIRE(scene.getParent(a).isNull());
        REQUIRE(scene.getParent(b) == a);
        REQUIRE(scene.getParent(c) == b);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("rebuild after reparenting", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity root1{ .index = 0, .version = 0 };
        Entity root2{ .index = 1, .version = 0 };
        Entity child{ .index = 2, .version = 0 };

        scene.track(root1, Transform{}, SceneGraphKey);
        scene.track(root2, Transform{}, SceneGraphKey);
        scene.track(child, Transform{}, SceneGraphKey);

        scene.setParent(child, root1, SceneGraphKey);
        scene.update(SceneGraphKey);

        REQUIRE(scene.getParent(child) == root1);
        REQUIRE(scene.getChildren(root1).size() == 1);

        // Reparent to root2
        scene.setParent(child, root2, SceneGraphKey);
        scene.update(SceneGraphKey);

        REQUIRE(scene.size() == 3);
        REQUIRE(scene.getParent(child) == root2);
        REQUIRE(scene.getChildren(root1).size() == 0);
        REQUIRE(scene.getChildren(root2).size() == 1);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("multiple independent roots", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity r1{ .index = 0, .version = 0 };
        Entity r2{ .index = 1, .version = 0 };
        Entity r3{ .index = 2, .version = 0 };

        scene.track(r1, Transform{}, SceneGraphKey);
        scene.track(r2, Transform{}, SceneGraphKey);
        scene.track(r3, Transform{}, SceneGraphKey);

        scene.update(SceneGraphKey);

        REQUIRE(scene.size() == 3);
        REQUIRE(scene.getParent(r1).isNull());
        REQUIRE(scene.getParent(r2).isNull());
        REQUIRE(scene.getParent(r3).isNull());
        REQUIRE(scene.getChildren(r1).size() == 0);
        REQUIRE(scene.getChildren(r2).size() == 0);
        REQUIRE(scene.getChildren(r3).size() == 0);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("wide hierarchy", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity parent{ .index = 0, .version = 0 };
        scene.track(parent, Transform{}, SceneGraphKey);

        constexpr uint32_t childCount = 50;
        std::vector<Entity> children;

        for (uint32_t i = 1; i <= childCount; ++i)
        {
            Entity child{ .index = i, .version = 0 };
            children.push_back(child);
            scene.track(child, Transform{}, SceneGraphKey);
            scene.setParent(child, parent, SceneGraphKey);
        }

        scene.update(SceneGraphKey);

        REQUIRE(scene.size() == childCount + 1);
        REQUIRE(scene.getChildren(parent).size() == childCount);

        for (auto const& child : children)
        {
            REQUIRE(scene.getParent(child) == parent);
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("deep hierarchy", "[core::scenegraph]")
    {
        SceneGraph scene;

        constexpr uint32_t depth = 20;
        std::vector<Entity> chain;

        for (uint32_t i = 0; i < depth; ++i)
        {
            Entity entity{ .index = i, .version = 0 };
            chain.push_back(entity);
            scene.track(entity, Transform{}, SceneGraphKey);

            if (i > 0)
            {
                scene.setParent(entity, chain[i - 1], SceneGraphKey);
            }
        }

        scene.update(SceneGraphKey);

        REQUIRE(scene.size() == depth);
        REQUIRE(scene.getParent(chain[0]).isNull());

        for (uint32_t i = 1; i < depth; ++i)
        {
            REQUIRE(scene.getParent(chain[i]) == chain[i - 1]);
            REQUIRE(scene.getChildren(chain[i - 1]).size() == 1);
            REQUIRE(scene.getChildren(chain[i - 1])[0] == chain[i]);
        }

        REQUIRE(scene.getChildren(chain[depth - 1]).size() == 0);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("cascade destroy removes entire subtree", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity root{ .index = 0, .version = 0 };
        Entity mid{ .index = 1, .version = 0 };
        Entity leaf1{ .index = 2, .version = 0 };
        Entity leaf2{ .index = 3, .version = 0 };
        Entity unrelated{ .index = 4, .version = 0 };

        scene.track(root, Transform{}, SceneGraphKey);
        scene.track(mid, Transform{}, SceneGraphKey);
        scene.track(leaf1, Transform{}, SceneGraphKey);
        scene.track(leaf2, Transform{}, SceneGraphKey);
        scene.track(unrelated, Transform{}, SceneGraphKey);

        scene.setParent(mid, root, SceneGraphKey);
        scene.setParent(leaf1, mid, SceneGraphKey);
        scene.setParent(leaf2, mid, SceneGraphKey);

        scene.untrackEntity(root, SceneGraphUntrackBehavior::Cascade, SceneGraphKey);

        REQUIRE(scene.size() == 1);
        REQUIRE(scene.isPresent(root) == false);
        REQUIRE(scene.isPresent(mid) == false);
        REQUIRE(scene.isPresent(leaf1) == false);
        REQUIRE(scene.isPresent(leaf2) == false);
        REQUIRE(scene.isPresent(unrelated) == true);

        scene.update(SceneGraphKey);
        REQUIRE(scene.size() == 1);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("orphan promotes children to roots", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity root{ .index = 0, .version = 0 };
        Entity childA{ .index = 1, .version = 0 };
        Entity childB{ .index = 2, .version = 0 };
        Entity grandchild{ .index = 3, .version = 0 };

        scene.track(root, Transform{}, SceneGraphKey);
        scene.track(childA, Transform{}, SceneGraphKey);
        scene.track(childB, Transform{}, SceneGraphKey);
        scene.track(grandchild, Transform{}, SceneGraphKey);

        scene.setParent(childA, root, SceneGraphKey);
        scene.setParent(childB, root, SceneGraphKey);
        scene.setParent(grandchild, childA, SceneGraphKey);

        // Orphan root's children — childA and childB become roots
        // but grandchild stays under childA
        scene.untrackEntity(root, SceneGraphUntrackBehavior::Orphan, SceneGraphKey);

        REQUIRE(scene.size() == 3);
        REQUIRE(scene.getParent(childA).isNull());
        REQUIRE(scene.getParent(childB).isNull());
        REQUIRE(scene.getParent(grandchild) == childA);

        scene.update(SceneGraphKey);
        REQUIRE(scene.size() == 3);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("entity version mismatch is not present", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity v0{ .index = 0, .version = 0 };
        Entity v1{ .index = 0, .version = 1 };

        scene.track(v0, Transform{}, SceneGraphKey);

        REQUIRE(scene.isPresent(v0) == true);
        REQUIRE(scene.isPresent(v1) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("entity slot reuse after untrack", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity v0{ .index = 0, .version = 0 };
        scene.track(v0, Transform{}, SceneGraphKey);
        scene.untrackEntity(v0, SceneGraphUntrackBehavior::Cascade, SceneGraphKey);

        REQUIRE(scene.isPresent(v0) == false);

        // Same index, new generation
        Entity v1{ .index = 0, .version = 1 };
        scene.track(v1, Transform{}, SceneGraphKey);

        REQUIRE(scene.size() == 1);
        REQUIRE(scene.isPresent(v1) == true);
        REQUIRE(scene.isPresent(v0) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("update with no changes is no-op", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity entity{ .index = 0, .version = 0 };
        scene.track(entity, Transform{}, SceneGraphKey);
        scene.update(SceneGraphKey);

        REQUIRE(scene.size() == 1);

        // Second update with no changes
        scene.update(SceneGraphKey);

        REQUIRE(scene.size() == 1);
        REQUIRE(scene.isPresent(entity) == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("getChildren returns empty for leaf node", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity leaf{ .index = 0, .version = 0 };
        scene.track(leaf, Transform{}, SceneGraphKey);

        std::vector<Entity> children = scene.getChildren(leaf);
        REQUIRE(children.empty());
    } LITL_END_TEST_CASE
}