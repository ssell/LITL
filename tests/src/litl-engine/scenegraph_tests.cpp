#include "tests.hpp"
#include "litl-engine/scene/scenegraph.hpp"

namespace litl::tests
{
    namespace
    {
        struct TestParentEntityWriteKey : ParentEntityWriteKey
        {
            TestParentEntityWriteKey() {}
        };

        static TestParentEntityWriteKey ParentWriteKey{};
    }

    LITL_TEST_CASE("add and remove", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity entity{ .index = 0, .version = 0 };
        Transform transform{};

        REQUIRE(scene.count() == 0);
        REQUIRE(scene.isPresent(entity) == false);
        
        scene.add(entity, transform);

        REQUIRE(scene.count() == 1);
        REQUIRE(scene.isPresent(entity) == true);

        // adding a second time should throw an assert
        LITL_START_ASSERT_CAPTURE
            scene.add(entity, transform);
        LITL_END_ASSERT_CAPTURE

        scene.remove(entity);

        REQUIRE(scene.count() == 0);
        REQUIRE(scene.isPresent(entity) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("add with parent", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity child{ .index = 0, .version = 0 };
        Entity parent{ .index = 1, .version = 0 };

        Transform childTransform{};
        Transform parentTransform{};

        childTransform.parent.set(parent, ParentWriteKey);

        // fail to add child with removed parent
        LITL_START_ASSERT_CAPTURE
            scene.add(child, childTransform);
        LITL_END_ASSERT_CAPTURE

        childTransform.parent.set(child, ParentWriteKey);

        // fail to add child with self parentage
        LITL_START_ASSERT_CAPTURE
            scene.add(child, childTransform);
        LITL_END_ASSERT_CAPTURE

        childTransform.parent.set(parent, ParentWriteKey);

        scene.add(parent, parentTransform);
        scene.add(child, childTransform);

        REQUIRE(scene.count() == 2);
        REQUIRE(scene.isPresent(child) == true);
        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.getParent(child) == parent);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("set parent", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity child{ .index = 0, .version = 0 };
        Entity parent{ .index = 1, .version = 0 };

        scene.add(child, Transform{});
        scene.add(parent, Transform{});

        REQUIRE(scene.count() == 2);
        REQUIRE(scene.isPresent(child) == true);
        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.getParent(child).isNull());

        LITL_START_ASSERT_CAPTURE
            scene.setParent(child, child);
        LITL_END_ASSERT_CAPTURE

        scene.setParent(child, parent);

        REQUIRE(scene.count() == 2);
        REQUIRE(scene.isPresent(child) == true);
        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.getParent(child) == parent);

        std::vector<Entity> children = scene.getChildren(parent);

        REQUIRE(children.size() == 1);
        REQUIRE(children[0] == child);

        scene.setParent(child, Entity::null());
        children = scene.getChildren(parent);

        REQUIRE(scene.count() == 2);
        REQUIRE(scene.isPresent(child));
        REQUIRE(scene.isPresent(parent));
        REQUIRE(scene.getParent(child).isNull());
        REQUIRE(children.size() == 0);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("remove", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity child{ .index = 0, .version = 0 };
        Entity parent{ .index = 1, .version = 0 };

        scene.add(child, Transform{});
        scene.add(parent, Transform{});
        scene.setParent(child, parent);

        REQUIRE(scene.count() == 2);
        REQUIRE(scene.isPresent(child) == true);
        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.getParent(child) == parent);

        scene.remove(child);

        REQUIRE(scene.count() == 1);
        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.isPresent(child) == false);

        LITL_START_ASSERT_CAPTURE
            REQUIRE(scene.getParent(child) == Entity::null());
        LITL_END_ASSERT_CAPTURE

        scene.add(child, Transform{});
        scene.setParent(child, parent);

        REQUIRE(scene.count() == 2);
        REQUIRE(scene.isPresent(child) == true);
        REQUIRE(scene.isPresent(parent) == true);
        REQUIRE(scene.getParent(child) == parent);

        scene.remove(parent);
        
        REQUIRE(scene.count() == 0);
        REQUIRE(scene.isPresent(child) == false);
        REQUIRE(scene.isPresent(parent) == false);

    } LITL_END_TEST_CASE

    LITL_TEST_CASE("rebuild produces valid topological order", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity a{ .index = 0, .version = 0 };  // root
        Entity b{ .index = 1, .version = 0 };  // child of a
        Entity c{ .index = 2, .version = 0 };  // child of b (grandchild of a)

        scene.add(a, Transform{});
        scene.add(b, Transform{});
        scene.add(c, Transform{});

        scene.setParent(b, a);
        scene.setParent(c, b);

        scene.update();

        // Tree should have all 3 nodes
        REQUIRE(scene.count() == 3);

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

        scene.add(root1, Transform{});
        scene.add(root2, Transform{});
        scene.add(child, Transform{});

        scene.setParent(child, root1);
        scene.update();

        REQUIRE(scene.getParent(child) == root1);
        REQUIRE(scene.getChildren(root1).size() == 1);

        // Reparent to root2
        scene.setParent(child, root2);
        scene.update();

        REQUIRE(scene.count() == 3);
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

        scene.add(r1, Transform{});
        scene.add(r2, Transform{});
        scene.add(r3, Transform{});

        scene.update();

        REQUIRE(scene.count() == 3);
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
        scene.add(parent, Transform{});

        constexpr uint32_t childCount = 50;
        std::vector<Entity> children;

        for (uint32_t i = 1; i <= childCount; ++i)
        {
            Entity child{ .index = i, .version = 0 };
            children.push_back(child);
            scene.add(child, Transform{});
            scene.setParent(child, parent);
        }

        scene.update();

        REQUIRE(scene.count() == childCount + 1);
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
            scene.add(entity, Transform{});

            if (i > 0)
            {
                scene.setParent(entity, chain[i - 1]);
            }
        }

        scene.update();

        REQUIRE(scene.count() == depth);
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

        scene.add(root, Transform{});
        scene.add(mid, Transform{});
        scene.add(leaf1, Transform{});
        scene.add(leaf2, Transform{});
        scene.add(unrelated, Transform{});

        scene.setParent(mid, root);
        scene.setParent(leaf1, mid);
        scene.setParent(leaf2, mid);

        scene.remove(root);

        REQUIRE(scene.count() == 1);
        REQUIRE(scene.isPresent(root) == false);
        REQUIRE(scene.isPresent(mid) == false);
        REQUIRE(scene.isPresent(leaf1) == false);
        REQUIRE(scene.isPresent(leaf2) == false);
        REQUIRE(scene.isPresent(unrelated) == true);

        scene.update();
        REQUIRE(scene.count() == 1);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("entity version mismatch is not present", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity v0{ .index = 0, .version = 0 };
        Entity v1{ .index = 0, .version = 1 };

        scene.add(v0, Transform{});

        REQUIRE(scene.isPresent(v0) == true);
        REQUIRE(scene.isPresent(v1) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("entity slot reuse after remove", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity v0{ .index = 0, .version = 0 };
        scene.add(v0, Transform{});
        scene.remove(v0);

        REQUIRE(scene.isPresent(v0) == false);

        // Same index, new generation
        Entity v1{ .index = 0, .version = 1 };
        scene.add(v1, Transform{});

        REQUIRE(scene.count() == 1);
        REQUIRE(scene.isPresent(v1) == true);
        REQUIRE(scene.isPresent(v0) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("update with no changes is no-op", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity entity{ .index = 0, .version = 0 };
        scene.add(entity, Transform{});
        scene.update();

        REQUIRE(scene.count() == 1);

        // Second update with no changes
        scene.update();

        REQUIRE(scene.count() == 1);
        REQUIRE(scene.isPresent(entity) == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("getChildren returns empty for leaf node", "[core::scenegraph]")
    {
        SceneGraph scene;

        Entity leaf{ .index = 0, .version = 0 };
        scene.add(leaf, Transform{});

        std::vector<Entity> children = scene.getChildren(leaf);
        REQUIRE(children.empty());
    } LITL_END_TEST_CASE
}