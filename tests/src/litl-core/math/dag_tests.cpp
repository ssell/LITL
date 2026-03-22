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
        // 2 has in-degree of 2
        // 3 has in-degree of 1
        // 4 has in-degree of 2

        // expected sort frontier of:
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
}