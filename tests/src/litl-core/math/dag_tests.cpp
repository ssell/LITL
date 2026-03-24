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

    LITL_TEST_CASE("addNodes", "[math::dag]")
    {
        DirectedAcyclicGraph dag;

        REQUIRE(dag.addNodes({ 0, 1, 2 }) == true);
        REQUIRE(dag.addNodes({ 0, 1, 2 }) == false);
        REQUIRE(dag.size() == 3);
        REQUIRE(dag.addNode(0) == false);
        REQUIRE(dag.size() == 3);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("addEdge", "[math::dag]")
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

        REQUIRE(dag.addEdge(0, 1) == false);        // graph is still empty

        REQUIRE(dag.addNode(0) == true);
        REQUIRE(dag.addNode(1) == true);
        REQUIRE(dag.addNode(2) == true);
        REQUIRE(dag.addNode(3) == true);
        REQUIRE(dag.addNode(4) == true);
        REQUIRE(dag.size() == 5);

        REQUIRE(dag.addEdge(0, 1) == true);
        REQUIRE(dag.addEdge(0, 2) == true);
        REQUIRE(dag.addEdge(1, 3) == true);
        REQUIRE(dag.addEdge(1, 4) == true);
        REQUIRE(dag.addEdge(2, 4) == true);

        REQUIRE(dag.addEdge(0, 1) == false);        // already exists

        REQUIRE(dag.containsEdge(0, 1) == true);
        REQUIRE(dag.containsEdge(1, 0) == false);   // dag is directed. 0 -> 1 != 0 <- 1

        REQUIRE(dag.hasIncoming(0) == false);
        REQUIRE(dag.hasOutgoing(0) == true);
        REQUIRE(dag.hasIncoming(1) == true);
        REQUIRE(dag.hasOutgoing(1) == true);
        REQUIRE(dag.hasIncoming(2) == true);
        REQUIRE(dag.hasOutgoing(2) == true);
        REQUIRE(dag.hasIncoming(3) == true);
        REQUIRE(dag.hasOutgoing(3) == false);
        REQUIRE(dag.hasIncoming(4) == true);
        REQUIRE(dag.hasOutgoing(4) == false);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("addEdges", "[math::dag]")
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

        REQUIRE(dag.addNodes({ 0, 1, 2, 3, 4 }) == true);
        REQUIRE(dag.size() == 5);

        REQUIRE(dag.addEdges({ {0, 1}, {0, 2}, {1, 3}, {1, 4}, {2, 4} }) == true);

        REQUIRE(dag.addEdge(0, 1) == false);        // already exists
        REQUIRE(dag.addEdges({ {0, 2}, {1, 3} }) == false);

        REQUIRE(dag.containsEdge(0, 1) == true);
        REQUIRE(dag.containsEdge(1, 0) == false);   // dag is directed. 0 -> 1 != 0 <- 1
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("sort 1", "[math::dag]")
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

        dag.addNodes({ 4, 3, 2, 1, 0 });
        dag.addEdges({ { 0, 1 }, { 0, 2 }, { 1, 3 }, { 1, 4 }, { 2, 4 } });

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

        dag.addNodes({ 0, 1, 2, 3, 4 });
        dag.addEdges({ {0, 4}, {4, 1}, {4, 2}, {1, 2}, {2, 3} });

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

        dag.addNodes({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
        dag.addEdges({ {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {1, 6}, {2, 6}, {3, 7}, {4, 8}, {5, 8}, {6, 9}, {7, 9}, {8, 9} });

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

    LITL_TEST_CASE("sort 4", "[math::dag]")
    {
        DirectedAcyclicGraph dag;

        /**
         *     0   1   2   3   4   10
         *     │   │   │   │   │   │
         *     └─┬─┘   │   └─┬─┘   │
         *       │     │     │     │
         *       V     V     V     V
         *       5     6     7     11
         *       │     │     │
         *       └─────┼─────┘
         *             │
         *             V
         *             8
         *             │
         *             V
         *             9
         * 
         * 0 -> 5
         * 1 -> 5
         * 2 -> 6
         * 3 -> 7
         * 4 -> 7
         * 5 -> 8
         * 6 -> 8
         * 7 -> 8
         * 8 -> 9
         * 10 -> 11
         */

        dag.addNodes({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 });
        dag.addEdges({ {0, 5}, {1, 5}, {2, 6}, {3, 7}, {4, 7}, {5, 8}, {6, 8}, {7, 8}, {8, 9}, {10, 11} });

        // expected layers of:
        // [0, 1, 2, 3, 4, 10]
        // [5, 6, 7, 11],
        // [8]
        // [9]

        REQUIRE(dag.sort() == true);

        auto& sorted = dag.getSorted();

        REQUIRE(sorted.size() == 12);
        REQUIRE(sorted[0] == 0);
        REQUIRE(sorted[1] == 1);
        REQUIRE(sorted[2] == 2);
        REQUIRE(sorted[3] == 3);
        REQUIRE(sorted[4] == 4);
        REQUIRE(sorted[5] == 10);
        REQUIRE(sorted[6] == 5);
        REQUIRE(sorted[7] == 6);
        REQUIRE(sorted[8] == 7);
        REQUIRE(sorted[9] == 11);
        REQUIRE(sorted[10] == 8);
        REQUIRE(sorted[11] == 9);

        auto& layers = dag.getLayers();

        REQUIRE(layers.size() == 4);
        REQUIRE(layers[0].size() == 6);
        REQUIRE(layers[0][0] == 0);
        REQUIRE(layers[0][1] == 1);
        REQUIRE(layers[0][2] == 2);
        REQUIRE(layers[0][3] == 3);
        REQUIRE(layers[0][4] == 4);
        REQUIRE(layers[0][5] == 10);
        REQUIRE(layers[1].size() == 4);
        REQUIRE(layers[1][0] == 5);
        REQUIRE(layers[1][1] == 6);
        REQUIRE(layers[1][2] == 7);
        REQUIRE(layers[1][3] == 11);
        REQUIRE(layers[2].size() == 1);
        REQUIRE(layers[2][0] == 8);
        REQUIRE(layers[3].size() == 1);
        REQUIRE(layers[3][0] == 9);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("cycles 1", "[math::dag]")
    {
        DirectedAcyclicGraph dag;

        /**
         *             0<──┐
         *             │   │
         *             V   │
         *             1───┘
         * 0 -> 1
         * 1 -> 0
         */

        REQUIRE(dag.addNodes({ 0, 1 }) == true);
        REQUIRE(dag.addEdges({ {0, 1}, {1, 0} }) == true);
        REQUIRE(dag.sort() == false);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("cycles 2", "[math::dag]")
    {
        DirectedAcyclicGraph dag;

        /**
         *               0
         *         ┌─────┴─────┐
         *         │           │
         *         │           │
         *         V           V
         *         1─────┐     2<──┐
         *         │     │     │   │
         *         │     │     │   │
         *         │     │     │   │
         *         V     │     V   │
         *         3     └───> 4───┘
         *
         * 0 -> 1
         * 0 -> 2
         * 1 -> 3
         * 1 -> 4
         * 2 -> 4
         * 4 -> 2
         */

        REQUIRE(dag.addNodes({ 4, 3, 2, 1, 0 }) == true);
        REQUIRE(dag.addEdges({ { 0, 1 }, { 0, 2 }, { 1, 3 }, { 1, 4 }, { 2, 4 }, {4, 2} }) == true);
        REQUIRE(dag.sort() == false);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("cycles 3", "[math::dag]")
    {
        DirectedAcyclicGraph dag;

        /**
         *               0
         *               │
         *               V
         *               1<──┐
         *               │   │
         *               V   │
         *               2   │
         *               │   │
         *               V   │
         *               3   │
         *               │   │
         *               V   │
         *               4───┘
         *               │
         *               V
         *               5
         * 0 -> 1
         * 1 -> 2
         * 2 -> 3
         * 3 -> 4
         * 4 -> 5
         * 4 -> 1
         */

        REQUIRE(dag.addNodes({ 0, 1, 2, 3, 4, 5 }) == true);
        REQUIRE(dag.addEdges({ {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {4, 1} }) == true);
        REQUIRE(dag.sort() == false);
    } END_LITL_TEST_CASE
}