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
}