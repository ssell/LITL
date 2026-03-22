#include "tests.hpp"
#include "litl-core/math/dag.hpp"

namespace LITL::Math::Tests
{
    LITL_TEST_CASE("addNode", "[math::dag]")
    {
        DirectedAcyclicGraph dag;

        DagNode node0 = 0;
        DagNode node1 = 1;
        DagNode node2 = 2;

        REQUIRE(dag.size() == 0);
        REQUIRE(dag.containsNode(node0) == false);
        REQUIRE(dag.containsNode(node1) == false);
        REQUIRE(dag.containsNode(node2) == false);

        REQUIRE(dag.addNode(node0) == true);
        REQUIRE(dag.size() == 1);
        REQUIRE(dag.containsNode(node0) == true);
        REQUIRE(dag.containsNode(node1) == false);
        REQUIRE(dag.containsNode(node2) == false);
        REQUIRE(dag.addNode(node0) == false);

        REQUIRE(dag.addNode(node1) == true);
        REQUIRE(dag.size() == 2);
        REQUIRE(dag.containsNode(node1) == true);
        REQUIRE(dag.containsNode(node1) == true);
        REQUIRE(dag.containsNode(node2) == false);
        REQUIRE(dag.addNode(node1) == false);

        REQUIRE(dag.addNode(node2) == true);
        REQUIRE(dag.size() == 3);
        REQUIRE(dag.containsNode(node0) == true);
        REQUIRE(dag.containsNode(node1) == true);
        REQUIRE(dag.containsNode(node2) == true);
        REQUIRE(dag.addNode(node2) == false);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("addEdge", "[math::dag]")
    {
        DirectedAcyclicGraph dag;

        DagNode node0 = 0;
        DagNode node1 = 1;
        DagNode node2 = 2;
        DagNode node3 = 3;
        DagNode node4 = 4;

        /**
         *               0
         *         ┌─────┴─────┐
         *         │           │
         *         │           │
         *         V           V
         *         1─────┐     2
         *         │     │     │
         *         │     │     │
         *         │     │     │
         *         V     │     V
         *         3     └───> 4
         * 
         * 0 -> 1
         * 0 -> 2
         * 1 -> 3
         * 1 -> 4
         * 2 -> 4
         */

        REQUIRE(dag.addEdge(node0, node1) == false);        // graph is still empty

        REQUIRE(dag.addNode(node0) == true);
        REQUIRE(dag.addNode(node1) == true);
        REQUIRE(dag.addNode(node2) == true);
        REQUIRE(dag.addNode(node3) == true);
        REQUIRE(dag.addNode(node4) == true);
        REQUIRE(dag.size() == 5);

        REQUIRE(dag.addEdge(node0, node1) == true);
        REQUIRE(dag.addEdge(node0, node2) == true);
        REQUIRE(dag.addEdge(node1, node3) == true);
        REQUIRE(dag.addEdge(node1, node4) == true);
        REQUIRE(dag.addEdge(node2, node4) == true);

        REQUIRE(dag.addEdge(node0, node1) == false);        // already exists

        REQUIRE(dag.containsEdge(node0, node1) == true);
        REQUIRE(dag.containsEdge(node1, node0) == false);   // dag is directed. 0 -> 1 != 0 <- 1

    } END_LITL_TEST_CASE

    LITL_TEST_CASE("sort", "[math::dag]")
    {
        DirectedAcyclicGraph dag;

        /**
         *               0
         *         ┌─────┴─────┐
         *         │           │
         *         │           │
         *         V           V
         *         1─────┐     2
         *         │     │     │
         *         │     │     │
         *         │     │     │
         *         V     │     V
         *         3     └───> 4
         *
         * 0 -> 1
         * 0 -> 2
         * 1 -> 3
         * 1 -> 4
         * 2 -> 4
         */

        dag.addNode(4); dag.addNode(3); dag.addNode(2); dag.addNode(1); dag.addNode(0);
        dag.addEdge(0, 1); dag.addEdge(0, 2);
        dag.addEdge(1, 3); dag.addEdge(1, 4);
        dag.addEdge(2, 4);

        // 0 has in-degree of 0
        // 1 has in-degree of 1
        // 2 has in-degree of 1
        // 3 has in-degree of 1
        // 4 has in-degree of 2

        // expected layers of:
        //     [0]
        //     [1, 2]
        //     [3, 4]

        REQUIRE(dag.sort() == true);

        auto& sorted = dag.getSorted();

        REQUIRE(sorted.size() == 5);
        REQUIRE(sorted[0] == 0);
        REQUIRE(sorted[1] == 1);
        REQUIRE(sorted[2] == 2);
        REQUIRE(sorted[3] == 3);
        REQUIRE(sorted[4] == 4);

        auto& layers = dag.getLayers();

        REQUIRE(layers.size() == 3);
        REQUIRE(layers[0].size() == 1);
        REQUIRE(layers[0][0] == 0);
        REQUIRE(layers[1].size() == 2);
        REQUIRE(layers[1][0] == 1);
        REQUIRE(layers[1][1] == 2);
        REQUIRE(layers[2].size() == 2);
        REQUIRE(layers[2][0] == 3);
        REQUIRE(layers[2][1] == 4);

    } END_LITL_TEST_CASE

    LITL_TEST_CASE("sort 2", "[math::dag]")
    {
        DirectedAcyclicGraph dag;
        
        /**
         *               0
         *               │
         *               V
         *        ┌───── 4 ─────┐
         *        │             │
         *        V             V
         *        1 ──────────> 2
         *                      │
         *                      V
         *                      3
         * 
         * 0 -> 4
         * 4 -> 1
         * 4 -> 2
         * 1 -> 2
         * 2 -> 3
         */

        dag.addNode(0); dag.addNode(1); dag.addNode(2); dag.addNode(3); dag.addNode(4);
        dag.addEdge(0, 4);
        dag.addEdge(4, 1);
        dag.addEdge(4, 2);
        dag.addEdge(1, 2);
        dag.addEdge(2, 3);

        // 0 has in-degree of 0
        // 1 has in-degree of 1
        // 2 has in-degree of 2
        // 3 has in-degree of 1
        // 4 has in-degree of 1

        // expected layers of:
        //     [0]
        //     [4]
        //     [1]
        //     [2]
        //     [3]

        REQUIRE(dag.sort() == true);

        auto& sorted = dag.getSorted();

        REQUIRE(sorted.size() == 5);
        REQUIRE(sorted[0] == 0);
        REQUIRE(sorted[1] == 4);
        REQUIRE(sorted[2] == 1);
        REQUIRE(sorted[3] == 2);
        REQUIRE(sorted[4] == 3);

        auto& layers = dag.getLayers();

        REQUIRE(layers.size() == 5);
        REQUIRE(layers[0].size() == 1);
        REQUIRE(layers[0][0] == 0);
        REQUIRE(layers[1].size() == 1);
        REQUIRE(layers[1][0] == 4);
        REQUIRE(layers[2].size() == 1);
        REQUIRE(layers[2][0] == 1);
        REQUIRE(layers[3].size() == 1);
        REQUIRE(layers[3][0] == 2);
        REQUIRE(layers[4].size() == 1);
        REQUIRE(layers[4][0] == 3);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("sort 3", "[math::dag]")
    {
        DirectedAcyclicGraph dag;

        /**
         *             0
         *             │
         *     ┌───┬── ┼ ──┬───┐
         *     │   │   │   │   │
         *     V   V   V   V   V
         *     1   2   3   4   5
         *     │   │   │   │   │
         *     └─┬─┘   │   └─┬─┘
         *       │     │     │
         *       V     V     V
         *       6     7     8
         *       │     │     │
         *       └─────┼─────┘
         *             │
         *             V
         *             9
         *
         * 0 -> 1
         * 0 -> 2
         * 0 -> 3
         * 0 -> 4
         * 0 -> 5
         * 1 -> 6
         * 2 -> 6
         * 3 -> 7
         * 4 -> 8
         * 5 -> 8
         * 6 -> 9
         * 7 -> 9
         * 8 -> 9
         */

        dag.addNode(0); dag.addNode(1); dag.addNode(2); dag.addNode(3); dag.addNode(4); dag.addNode(5); dag.addNode(6); dag.addNode(7); dag.addNode(8); dag.addNode(9);
        dag.addEdge(0, 1); dag.addEdge(0, 2); dag.addEdge(0, 3); dag.addEdge(0, 4); dag.addEdge(0, 5);
        dag.addEdge(1, 6);
        dag.addEdge(2, 6);
        dag.addEdge(3, 7);
        dag.addEdge(4, 8);
        dag.addEdge(5, 8);
        dag.addEdge(6, 9);
        dag.addEdge(7, 9);
        dag.addEdge(8, 9);

        // 0 has in-degree of 0
        // 1 has in-degree of 1
        // 2 has in-degree of 1
        // 3 has in-degree of 1
        // 4 has in-degree of 1
        // 5 has in-degree of 1
        // 6 has in-degree of 2
        // 7 has in-degree of 1
        // 8 has in-degree of 2
        // 9 has in-degree of 3

        // expected layers of:
        //     [0]
        //     [1, 2, 3, 4, 5]
        //     [6, 7, 8]
        //     [9]

        REQUIRE(dag.sort() == true);

        auto& sorted = dag.getSorted();

        REQUIRE(sorted.size() == 10);
        REQUIRE(sorted[0] == 0);
        REQUIRE(sorted[1] == 1);
        REQUIRE(sorted[2] == 2);
        REQUIRE(sorted[3] == 3);
        REQUIRE(sorted[4] == 4);
        REQUIRE(sorted[5] == 5);
        REQUIRE(sorted[6] == 6);
        REQUIRE(sorted[7] == 7);
        REQUIRE(sorted[8] == 8);
        REQUIRE(sorted[9] == 9);

        auto& layers = dag.getLayers();

        REQUIRE(layers.size() == 4);
        REQUIRE(layers[0].size() == 1);
        REQUIRE(layers[0][0] == 0);
        REQUIRE(layers[1].size() == 5);
        REQUIRE(layers[1][0] == 1);
        REQUIRE(layers[1][1] == 2);
        REQUIRE(layers[1][2] == 3);
        REQUIRE(layers[1][3] == 4);
        REQUIRE(layers[1][4] == 5);
        REQUIRE(layers[2].size() == 3);
        REQUIRE(layers[2][0] == 6);
        REQUIRE(layers[2][1] == 7);
        REQUIRE(layers[2][2] == 8);
        REQUIRE(layers[3].size() == 1);
        REQUIRE(layers[3][0] == 9);
    } END_LITL_TEST_CASE
}