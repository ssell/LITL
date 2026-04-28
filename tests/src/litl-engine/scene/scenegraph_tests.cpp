#include "tests.hpp"
#include "litl-engine/scene/sceneGraph.hpp"

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

    LITL_TEST_CASE("add and remove", "[engine::scenegraph]")
    {
        SceneGraph sceneGraph;

        Entity entity{ .index = 0, .version = 0 };
        Transform transform{};

        REQUIRE(sceneGraph.count() == 0);
        REQUIRE(sceneGraph.isPresent(entity) == false);
        
        sceneGraph.add(entity, transform);

        REQUIRE(sceneGraph.count() == 1);
        REQUIRE(sceneGraph.isPresent(entity) == true);

        // adding a second time should throw an assert
        LITL_START_ASSERT_CAPTURE
            sceneGraph.add(entity, transform);
        LITL_END_ASSERT_CAPTURE

        sceneGraph.remove(entity);

        REQUIRE(sceneGraph.count() == 0);
        REQUIRE(sceneGraph.isPresent(entity) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("add with parent", "[engine::scenegraph]")
    {
        SceneGraph sceneGraph;

        Entity child{ .index = 0, .version = 0 };
        Entity parent{ .index = 1, .version = 0 };

        Transform childTransform{};
        Transform parentTransform{};

        childTransform.parent.set(parent, ParentWriteKey);

        // fail to add child with removed parent
        LITL_START_ASSERT_CAPTURE
            sceneGraph.add(child, childTransform);
        LITL_END_ASSERT_CAPTURE

        childTransform.parent.set(child, ParentWriteKey);

        // fail to add child with self parentage
        LITL_START_ASSERT_CAPTURE
            sceneGraph.add(child, childTransform);
        LITL_END_ASSERT_CAPTURE

        childTransform.parent.set(parent, ParentWriteKey);

        sceneGraph.add(parent, parentTransform);
        sceneGraph.add(child, childTransform);

        REQUIRE(sceneGraph.count() == 2);
        REQUIRE(sceneGraph.isPresent(child) == true);
        REQUIRE(sceneGraph.isPresent(parent) == true);
        REQUIRE(sceneGraph.getParent(child) == parent);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("set parent", "[engine::scenegraph]")
    {
        SceneGraph sceneGraph;

        Entity child{ .index = 0, .version = 0 };
        Entity parent{ .index = 1, .version = 0 };

        sceneGraph.add(child, Transform{});
        sceneGraph.add(parent, Transform{});

        REQUIRE(sceneGraph.count() == 2);
        REQUIRE(sceneGraph.isPresent(child) == true);
        REQUIRE(sceneGraph.isPresent(parent) == true);
        REQUIRE(sceneGraph.getParent(child).isNull());

        LITL_START_ASSERT_CAPTURE
            sceneGraph.setParent(child, child);
        LITL_END_ASSERT_CAPTURE

        sceneGraph.setParent(child, parent);

        REQUIRE(sceneGraph.count() == 2);
        REQUIRE(sceneGraph.isPresent(child) == true);
        REQUIRE(sceneGraph.isPresent(parent) == true);
        REQUIRE(sceneGraph.getParent(child) == parent);

        std::vector<Entity> children = sceneGraph.getChildren(parent);

        REQUIRE(children.size() == 1);
        REQUIRE(children[0] == child);

        sceneGraph.setParent(child, Entity::null());
        children = sceneGraph.getChildren(parent);

        REQUIRE(sceneGraph.count() == 2);
        REQUIRE(sceneGraph.isPresent(child));
        REQUIRE(sceneGraph.isPresent(parent));
        REQUIRE(sceneGraph.getParent(child).isNull());
        REQUIRE(children.size() == 0);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("remove", "[engine::scenegraph]")
    {
        SceneGraph sceneGraph;

        Entity child{ .index = 0, .version = 0 };
        Entity parent{ .index = 1, .version = 0 };

        sceneGraph.add(child, Transform{});
        sceneGraph.add(parent, Transform{});
        sceneGraph.setParent(child, parent);

        REQUIRE(sceneGraph.count() == 2);
        REQUIRE(sceneGraph.isPresent(child) == true);
        REQUIRE(sceneGraph.isPresent(parent) == true);
        REQUIRE(sceneGraph.getParent(child) == parent);

        sceneGraph.remove(child);

        REQUIRE(sceneGraph.count() == 1);
        REQUIRE(sceneGraph.isPresent(parent) == true);
        REQUIRE(sceneGraph.isPresent(child) == false);

        LITL_START_ASSERT_CAPTURE
            REQUIRE(sceneGraph.getParent(child) == Entity::null());
        LITL_END_ASSERT_CAPTURE

        sceneGraph.add(child, Transform{});
        sceneGraph.setParent(child, parent);

        REQUIRE(sceneGraph.count() == 2);
        REQUIRE(sceneGraph.isPresent(child) == true);
        REQUIRE(sceneGraph.isPresent(parent) == true);
        REQUIRE(sceneGraph.getParent(child) == parent);

        sceneGraph.remove(parent);
        
        REQUIRE(sceneGraph.count() == 0);
        REQUIRE(sceneGraph.isPresent(child) == false);
        REQUIRE(sceneGraph.isPresent(parent) == false);

    } LITL_END_TEST_CASE

    LITL_TEST_CASE("rebuild produces valid topological order", "[engine::scenegraph]")
    {
        SceneGraph sceneGraph;

        Entity a{ .index = 0, .version = 0 };  // root
        Entity b{ .index = 1, .version = 0 };  // child of a
        Entity c{ .index = 2, .version = 0 };  // child of b (grandchild of a)

        sceneGraph.add(a, Transform{});
        sceneGraph.add(b, Transform{});
        sceneGraph.add(c, Transform{});

        sceneGraph.setParent(b, a);
        sceneGraph.setParent(c, b);

        sceneGraph.update();

        // Tree should have all 3 nodes
        REQUIRE(sceneGraph.count() == 3);

        // Verify parent-before-child invariant through getParent
        // After update, depths and sorted order should be consistent
        REQUIRE(sceneGraph.getParent(a).isNull());
        REQUIRE(sceneGraph.getParent(b) == a);
        REQUIRE(sceneGraph.getParent(c) == b);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("rebuild after reparenting", "[engine::scenegraph]")
    {
        SceneGraph sceneGraph;

        Entity root1{ .index = 0, .version = 0 };
        Entity root2{ .index = 1, .version = 0 };
        Entity child{ .index = 2, .version = 0 };

        sceneGraph.add(root1, Transform{});
        sceneGraph.add(root2, Transform{});
        sceneGraph.add(child, Transform{});

        sceneGraph.setParent(child, root1);
        sceneGraph.update();

        REQUIRE(sceneGraph.getParent(child) == root1);
        REQUIRE(sceneGraph.getChildren(root1).size() == 1);

        // Reparent to root2
        sceneGraph.setParent(child, root2);
        sceneGraph.update();

        REQUIRE(sceneGraph.count() == 3);
        REQUIRE(sceneGraph.getParent(child) == root2);
        REQUIRE(sceneGraph.getChildren(root1).size() == 0);
        REQUIRE(sceneGraph.getChildren(root2).size() == 1);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("multiple independent roots", "[engine::scenegraph]")
    {
        SceneGraph sceneGraph;

        Entity r1{ .index = 0, .version = 0 };
        Entity r2{ .index = 1, .version = 0 };
        Entity r3{ .index = 2, .version = 0 };

        sceneGraph.add(r1, Transform{});
        sceneGraph.add(r2, Transform{});
        sceneGraph.add(r3, Transform{});

        sceneGraph.update();

        REQUIRE(sceneGraph.count() == 3);
        REQUIRE(sceneGraph.getParent(r1).isNull());
        REQUIRE(sceneGraph.getParent(r2).isNull());
        REQUIRE(sceneGraph.getParent(r3).isNull());
        REQUIRE(sceneGraph.getChildren(r1).size() == 0);
        REQUIRE(sceneGraph.getChildren(r2).size() == 0);
        REQUIRE(sceneGraph.getChildren(r3).size() == 0);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("wide hierarchy", "[engine::scenegraph]")
    {
        SceneGraph sceneGraph;

        Entity parent{ .index = 0, .version = 0 };
        sceneGraph.add(parent, Transform{});

        constexpr uint32_t childCount = 50;
        std::vector<Entity> children;

        for (uint32_t i = 1; i <= childCount; ++i)
        {
            Entity child{ .index = i, .version = 0 };
            children.push_back(child);
            sceneGraph.add(child, Transform{});
            sceneGraph.setParent(child, parent);
        }

        sceneGraph.update();

        REQUIRE(sceneGraph.count() == childCount + 1);
        REQUIRE(sceneGraph.getChildren(parent).size() == childCount);

        for (auto const& child : children)
        {
            REQUIRE(sceneGraph.getParent(child) == parent);
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("deep hierarchy", "[engine::scenegraph]")
    {
        SceneGraph sceneGraph;

        constexpr uint32_t depth = 20;
        std::vector<Entity> chain;

        for (uint32_t i = 0; i < depth; ++i)
        {
            Entity entity{ .index = i, .version = 0 };
            chain.push_back(entity);
            sceneGraph.add(entity, Transform{});

            if (i > 0)
            {
                sceneGraph.setParent(entity, chain[i - 1]);
            }
        }

        sceneGraph.update();

        REQUIRE(sceneGraph.count() == depth);
        REQUIRE(sceneGraph.getParent(chain[0]).isNull());

        for (uint32_t i = 1; i < depth; ++i)
        {
            REQUIRE(sceneGraph.getParent(chain[i]) == chain[i - 1]);
            REQUIRE(sceneGraph.getChildren(chain[i - 1]).size() == 1);
            REQUIRE(sceneGraph.getChildren(chain[i - 1])[0] == chain[i]);
        }

        REQUIRE(sceneGraph.getChildren(chain[depth - 1]).size() == 0);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("cascade destroy removes entire subtree", "[engine::scenegraph]")
    {
        SceneGraph sceneGraph;

        Entity root{ .index = 0, .version = 0 };
        Entity mid{ .index = 1, .version = 0 };
        Entity leaf1{ .index = 2, .version = 0 };
        Entity leaf2{ .index = 3, .version = 0 };
        Entity unrelated{ .index = 4, .version = 0 };

        sceneGraph.add(root, Transform{});
        sceneGraph.add(mid, Transform{});
        sceneGraph.add(leaf1, Transform{});
        sceneGraph.add(leaf2, Transform{});
        sceneGraph.add(unrelated, Transform{});

        sceneGraph.setParent(mid, root);
        sceneGraph.setParent(leaf1, mid);
        sceneGraph.setParent(leaf2, mid);

        sceneGraph.remove(root);

        REQUIRE(sceneGraph.count() == 1);
        REQUIRE(sceneGraph.isPresent(root) == false);
        REQUIRE(sceneGraph.isPresent(mid) == false);
        REQUIRE(sceneGraph.isPresent(leaf1) == false);
        REQUIRE(sceneGraph.isPresent(leaf2) == false);
        REQUIRE(sceneGraph.isPresent(unrelated) == true);

        sceneGraph.update();
        REQUIRE(sceneGraph.count() == 1);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("entity version mismatch is not present", "[engine::scenegraph]")
    {
        SceneGraph sceneGraph;

        Entity v0{ .index = 0, .version = 0 };
        Entity v1{ .index = 0, .version = 1 };

        sceneGraph.add(v0, Transform{});

        REQUIRE(sceneGraph.isPresent(v0) == true);
        REQUIRE(sceneGraph.isPresent(v1) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("entity slot reuse after remove", "[engine::scenegraph]")
    {
        SceneGraph sceneGraph;

        Entity v0{ .index = 0, .version = 0 };
        sceneGraph.add(v0, Transform{});
        sceneGraph.remove(v0);

        REQUIRE(sceneGraph.isPresent(v0) == false);

        // Same index, new generation
        Entity v1{ .index = 0, .version = 1 };
        sceneGraph.add(v1, Transform{});

        REQUIRE(sceneGraph.count() == 1);
        REQUIRE(sceneGraph.isPresent(v1) == true);
        REQUIRE(sceneGraph.isPresent(v0) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("update with no changes is no-op", "[engine::scenegraph]")
    {
        SceneGraph sceneGraph;

        Entity entity{ .index = 0, .version = 0 };
        sceneGraph.add(entity, Transform{});
        sceneGraph.update();

        REQUIRE(sceneGraph.count() == 1);

        // Second update with no changes
        sceneGraph.update();

        REQUIRE(sceneGraph.count() == 1);
        REQUIRE(sceneGraph.isPresent(entity) == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("getChildren returns empty for leaf node", "[engine::scenegraph]")
    {
        SceneGraph sceneGraph;

        Entity leaf{ .index = 0, .version = 0 };
        sceneGraph.add(leaf, Transform{});

        std::vector<Entity> children = sceneGraph.getChildren(leaf);
        REQUIRE(children.empty());
    } LITL_END_TEST_CASE
}