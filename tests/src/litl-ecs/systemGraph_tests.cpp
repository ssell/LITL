#include "tests.hpp"
#include "litl-ecs/system/systemGraph.hpp"

namespace LITL::ECS::Tests
{
    LITL_TEST_CASE("Explicit Dependency", "[ecs::systemGraph]")
    {
        // Basic explicit dependency checks. The DAG-specific tests (which SystemGraph is built on) have more complex tests.
        SystemGraph systemGraph;

        // 3 depends on 2 which depends on 1. None depend on 0.
        // Expect to see layers:
        // [0, 1]
        // [2]
        // [3]

        systemGraph.add(0, {});
        systemGraph.add(1, {});
        systemGraph.add(2, {});
        systemGraph.add(3, {});

        REQUIRE(systemGraph.addDependency(3, 2) == true);
        REQUIRE(systemGraph.addDependency(2, 1) == true);
        REQUIRE(systemGraph.addDependency(8, 6) == false);      // neither 8 or 6 exist
        REQUIRE(systemGraph.addDependency(10, 0) == false);     // 10 does not exist

        REQUIRE(systemGraph.build() == true);

        auto& dag = systemGraph.getNodeGraph();
        auto& sorted = dag.getSorted();
        auto& layers = dag.getLayers();

        REQUIRE(sorted.size() == 4);
        REQUIRE(systemGraph.getNode(sorted[0]).systemId == 0);
        REQUIRE(systemGraph.getNode(sorted[1]).systemId == 1);
        REQUIRE(systemGraph.getNode(sorted[2]).systemId == 2);
        REQUIRE(systemGraph.getNode(sorted[3]).systemId == 3);

        REQUIRE(layers.size() == 3);
        REQUIRE(layers[0].size() == 2);
        REQUIRE(systemGraph.getNode(layers[0][0]).systemId == 0);
        REQUIRE(systemGraph.getNode(layers[0][1]).systemId == 1);
        REQUIRE(layers[1].size() == 1);
        REQUIRE(systemGraph.getNode(layers[1][0]).systemId == 2);
        REQUIRE(layers[2].size() == 1);
        REQUIRE(systemGraph.getNode(layers[2][0]).systemId == 3);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Implicit Dependency", "[ecs::systemGraph]")
    {
        // Implicit dependency checks - those based on component read/write access.
        SystemGraph systemGraph;

        // 0 writes to 0
        // 1 reads 0
        // 2 neither reads nor writes to 0
        // 3 reads 0
        // 4 writes 0
        // 5 neither reads nor writes to 0
        // 6 write to 0
        // 7 reads 0

        systemGraph.add(0, { { 0, false} });
        systemGraph.add(1, { { 0, true } });
        systemGraph.add(2, {});
        systemGraph.add(3, { { 0, true } });
        systemGraph.add(4, { { 0, false } });
        systemGraph.add(5, {});
        systemGraph.add(6, { { 0, false } });
        systemGraph.add(7, { { 0, true } });

        // Exported sort:
        // [0, 2, 5, 1, 3, 4, 6, 7]
        
        // Expected layers
        // [0, 2, 5]
        // [1, 3]
        // [4]
        // [6]
        // [7]

        REQUIRE(systemGraph.build() == true);

        auto& dag = systemGraph.getNodeGraph();
        auto& sorted = dag.getSorted();
        auto& layers = dag.getLayers();

        REQUIRE(sorted.size() == 8);
        REQUIRE(systemGraph.getNode(sorted[0]).systemId == 0);
        REQUIRE(systemGraph.getNode(sorted[1]).systemId == 2);
        REQUIRE(systemGraph.getNode(sorted[2]).systemId == 5);
        REQUIRE(systemGraph.getNode(sorted[3]).systemId == 1);
        REQUIRE(systemGraph.getNode(sorted[4]).systemId == 3);
        REQUIRE(systemGraph.getNode(sorted[5]).systemId == 4);
        REQUIRE(systemGraph.getNode(sorted[6]).systemId == 6);
        REQUIRE(systemGraph.getNode(sorted[7]).systemId == 7);

        REQUIRE(layers.size() == 5);
        REQUIRE(layers[0].size() == 3);
        REQUIRE(systemGraph.getNode(layers[0][0]).systemId == 0);
        REQUIRE(systemGraph.getNode(layers[0][1]).systemId == 2);
        REQUIRE(systemGraph.getNode(layers[0][2]).systemId == 5);
        REQUIRE(layers[1].size() == 2);
        REQUIRE(systemGraph.getNode(layers[1][0]).systemId == 1);
        REQUIRE(systemGraph.getNode(layers[1][1]).systemId == 3);
        REQUIRE(layers[2].size() == 1);
        REQUIRE(systemGraph.getNode(layers[2][0]).systemId == 4);
        REQUIRE(layers[3].size() == 1);
        REQUIRE(systemGraph.getNode(layers[3][0]).systemId == 6);
        REQUIRE(layers[4].size() == 1);
        REQUIRE(systemGraph.getNode(layers[4][0]).systemId == 7);

    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Prefer First", "[ecs::systemGraph]")
    {
        SystemGraph systemGraph;

        systemGraph.add(0, {});
        systemGraph.add(1, {});
        systemGraph.add(2, {});
        systemGraph.add(3, {});
        systemGraph.add(4, {});

        REQUIRE(systemGraph.setPlacementHint(2, SystemPlacementHint::First) == true);
        REQUIRE(systemGraph.setPlacementHint(4, SystemPlacementHint::First) == true);
        REQUIRE(systemGraph.setPlacementHint(9, SystemPlacementHint::First) == false);

        // Expected sort:
        // [2, 4]               <-- the explicit first systems
        // [0, 1, 3]            <-- everyone else

        REQUIRE(systemGraph.build() == true);

        auto& dag = systemGraph.getNodeGraph();
        auto& sorted = dag.getSorted();
        auto& layers = dag.getLayers();

        REQUIRE(sorted.size() == 5);
        REQUIRE(systemGraph.getNode(sorted[0]).systemId == 2);
        REQUIRE(systemGraph.getNode(sorted[1]).systemId == 4);
        REQUIRE(systemGraph.getNode(sorted[2]).systemId == 0);
        REQUIRE(systemGraph.getNode(sorted[3]).systemId == 1);
        REQUIRE(systemGraph.getNode(sorted[4]).systemId == 3);

        REQUIRE(layers.size() == 2);
        REQUIRE(layers[0].size() == 2);
        REQUIRE(systemGraph.getNode(layers[0][0]).systemId == 2);
        REQUIRE(systemGraph.getNode(layers[0][1]).systemId == 4);
        REQUIRE(layers[1].size() == 3);
        REQUIRE(systemGraph.getNode(layers[1][0]).systemId == 0);
        REQUIRE(systemGraph.getNode(layers[1][1]).systemId == 1);
        REQUIRE(systemGraph.getNode(layers[1][2]).systemId == 3);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Prefer Last", "[ecs::systemGraph]")
    {
        SystemGraph systemGraph;

        systemGraph.add(0, {});
        systemGraph.add(1, {});
        systemGraph.add(2, {});
        systemGraph.add(3, {});
        systemGraph.add(4, {});

        REQUIRE(systemGraph.setPlacementHint(2, SystemPlacementHint::Last) == true);
        REQUIRE(systemGraph.setPlacementHint(4, SystemPlacementHint::Last) == true);
        REQUIRE(systemGraph.setPlacementHint(9, SystemPlacementHint::Last) == false);

        // Expected sort:
        // [0, 1, 3]            <-- everyone else
        // [2, 4]               <-- the explicit last systems

        REQUIRE(systemGraph.build() == true);

        auto& dag = systemGraph.getNodeGraph();
        auto& sorted = dag.getSorted();
        auto& layers = dag.getLayers();

        REQUIRE(sorted.size() == 5);
        REQUIRE(systemGraph.getNode(sorted[0]).systemId == 0);
        REQUIRE(systemGraph.getNode(sorted[1]).systemId == 1);
        REQUIRE(systemGraph.getNode(sorted[2]).systemId == 3);
        REQUIRE(systemGraph.getNode(sorted[3]).systemId == 2);
        REQUIRE(systemGraph.getNode(sorted[4]).systemId == 4);

        REQUIRE(layers.size() == 2);
        REQUIRE(layers[0].size() == 3);
        REQUIRE(systemGraph.getNode(layers[0][0]).systemId == 0);
        REQUIRE(systemGraph.getNode(layers[0][1]).systemId == 1);
        REQUIRE(systemGraph.getNode(layers[0][2]).systemId == 3);
        REQUIRE(layers[1].size() == 2);
        REQUIRE(systemGraph.getNode(layers[1][0]).systemId == 2);
        REQUIRE(systemGraph.getNode(layers[1][1]).systemId == 4);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Mixed Dependency", "[ecs::systemGraph]")
    {
        // Mix of explicit and implicit dependencies along with placement hints. Fun.
        SystemGraph systemGraph;

        constexpr uint32_t PhysicsSystem = 0;
        constexpr uint32_t MovementSystem = 1;
        constexpr uint32_t CameraFollowSystem = 2;
        constexpr uint32_t FallDamageSystem = 3;
        constexpr uint32_t AnimationStateSystem = 4;
        constexpr uint32_t NetworkReceiveSystem = 5;
        constexpr uint32_t NetworkSendSystem = 6;

        constexpr uint32_t TransformComponent = 0;
        constexpr uint32_t VelocityComponent = 1;

        constexpr bool ReadOnly = true;
        constexpr bool ReadWrite = false;

        systemGraph.add(PhysicsSystem, { {TransformComponent, ReadOnly}, {VelocityComponent, ReadWrite} });
        systemGraph.add(MovementSystem, { {TransformComponent, ReadWrite}, {VelocityComponent, ReadOnly} });
        systemGraph.add(NetworkReceiveSystem, { {TransformComponent, ReadWrite}, {VelocityComponent, ReadWrite} });
        systemGraph.add(NetworkSendSystem, { {TransformComponent, ReadWrite}, {VelocityComponent, ReadWrite} });
        systemGraph.add(CameraFollowSystem, { {TransformComponent, ReadOnly} });
        systemGraph.add(FallDamageSystem, { {TransformComponent, ReadOnly}, {VelocityComponent, ReadOnly} });
        systemGraph.add(AnimationStateSystem, { {TransformComponent, ReadOnly} });

        systemGraph.addDependency(MovementSystem, PhysicsSystem);
        systemGraph.addDependency(CameraFollowSystem, MovementSystem);
        systemGraph.addDependency(FallDamageSystem, PhysicsSystem);

        systemGraph.setPlacementHint(PhysicsSystem, SystemPlacementHint::First);
        systemGraph.setPlacementHint(NetworkReceiveSystem, SystemPlacementHint::First);
        systemGraph.setPlacementHint(NetworkSendSystem, SystemPlacementHint::Last);

        REQUIRE(systemGraph.build() == true);

        auto& dag = systemGraph.getNodeGraph();
        auto& sorted = dag.getSorted();
        auto& layers = dag.getLayers();

        REQUIRE(sorted.size() == 7);
        REQUIRE(systemGraph.getNode(sorted[0]).systemId == PhysicsSystem);          // remember: tie-breaker goes to the system added first
        REQUIRE(systemGraph.getNode(sorted[1]).systemId == NetworkReceiveSystem);
        REQUIRE(systemGraph.getNode(sorted[2]).systemId == MovementSystem);
        REQUIRE(systemGraph.getNode(sorted[3]).systemId == CameraFollowSystem);
        REQUIRE(systemGraph.getNode(sorted[4]).systemId == FallDamageSystem);
        REQUIRE(systemGraph.getNode(sorted[5]).systemId == AnimationStateSystem);
        REQUIRE(systemGraph.getNode(sorted[6]).systemId == NetworkSendSystem);

        REQUIRE(layers.size() == 5);
        REQUIRE(layers[0].size() == 1);
        REQUIRE(systemGraph.getNode(layers[0][0]).systemId == PhysicsSystem);
        REQUIRE(layers[1].size() == 1);
        REQUIRE(systemGraph.getNode(layers[1][0]).systemId == NetworkReceiveSystem);
        REQUIRE(layers[2].size() == 1);
        REQUIRE(systemGraph.getNode(layers[2][0]).systemId == MovementSystem);
        REQUIRE(layers[3].size() == 3);
        REQUIRE(systemGraph.getNode(layers[3][0]).systemId == CameraFollowSystem);
        REQUIRE(systemGraph.getNode(layers[3][1]).systemId == FallDamageSystem);
        REQUIRE(systemGraph.getNode(layers[3][2]).systemId == AnimationStateSystem);
        REQUIRE(layers[4].size() == 1);
        REQUIRE(systemGraph.getNode(layers[4][0]).systemId == NetworkSendSystem);
    } END_LITL_TEST_CASE
}