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
        REQUIRE(sorted[0] == 0);
        REQUIRE(sorted[1] == 1);
        REQUIRE(sorted[2] == 2);
        REQUIRE(sorted[3] == 3);

        REQUIRE(layers.size() == 3);
        REQUIRE(layers[0].size() == 2);
        REQUIRE(layers[0][0] == 0);
        REQUIRE(layers[0][1] == 1);
        REQUIRE(layers[1].size() == 1);
        REQUIRE(layers[1][0] == 2);
        REQUIRE(layers[2].size() == 1);
        REQUIRE(layers[2][0] == 3);
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
        REQUIRE(sorted[0] == 0);
        REQUIRE(sorted[1] == 2);
        REQUIRE(sorted[2] == 5);
        REQUIRE(sorted[3] == 1);
        REQUIRE(sorted[4] == 3);
        REQUIRE(sorted[5] == 4);
        REQUIRE(sorted[6] == 6);
        REQUIRE(sorted[7] == 7);

        REQUIRE(layers.size() == 5);
        REQUIRE(layers[0].size() == 3);
        REQUIRE(layers[0][0] == 0);
        REQUIRE(layers[0][1] == 2);
        REQUIRE(layers[0][2] == 5);
        REQUIRE(layers[1].size() == 2);
        REQUIRE(layers[1][0] == 1);
        REQUIRE(layers[1][1] == 3);
        REQUIRE(layers[2].size() == 1);
        REQUIRE(layers[2][0] == 4);
        REQUIRE(layers[3].size() == 1);
        REQUIRE(layers[3][0] == 6);
        REQUIRE(layers[4].size() == 1);
        REQUIRE(layers[4][0] == 7);

    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Prefer First", "[ecs::systemGraph]")
    {
        SystemGraph systemGraph;

        systemGraph.add(0, {});
        systemGraph.add(1, {});
        systemGraph.add(2, {});
        systemGraph.add(3, {});
        systemGraph.add(4, {});

        REQUIRE(systemGraph.setPlacementHint(2, SystemNodePlacementHint::First) == true);
        REQUIRE(systemGraph.setPlacementHint(4, SystemNodePlacementHint::First) == true);
        REQUIRE(systemGraph.setPlacementHint(9, SystemNodePlacementHint::First) == false);

        // Expected sort:
        // [2, 4]               <-- the explicit first systems
        // [0, 1, 3]            <-- everyone else

        REQUIRE(systemGraph.build() == true);

        auto& dag = systemGraph.getNodeGraph();
        auto& sorted = dag.getSorted();
        auto& layers = dag.getLayers();

        REQUIRE(sorted.size() == 5);
        REQUIRE(sorted[0] == 2);
        REQUIRE(sorted[1] == 4);
        REQUIRE(sorted[2] == 0);
        REQUIRE(sorted[3] == 1);
        REQUIRE(sorted[4] == 3);
        REQUIRE(layers.size() == 2);
        REQUIRE(layers[0].size() == 2);
        REQUIRE(layers[0][0] == 2);
        REQUIRE(layers[0][1] == 4);
        REQUIRE(layers[1].size() == 3);
        REQUIRE(layers[1][0] == 0);
        REQUIRE(layers[1][1] == 1);
        REQUIRE(layers[1][2] == 3);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Prefer Last", "[ecs::systemGraph]")
    {
        SystemGraph systemGraph;

        systemGraph.add(0, {});
        systemGraph.add(1, {});
        systemGraph.add(2, {});
        systemGraph.add(3, {});
        systemGraph.add(4, {});

        REQUIRE(systemGraph.setPlacementHint(2, SystemNodePlacementHint::Last) == true);
        REQUIRE(systemGraph.setPlacementHint(4, SystemNodePlacementHint::Last) == true);
        REQUIRE(systemGraph.setPlacementHint(9, SystemNodePlacementHint::Last) == false);

        // Expected sort:
        // [0, 1, 3]            <-- everyone else
        // [2, 4]               <-- the explicit last systems

        REQUIRE(systemGraph.build() == true);

        auto& dag = systemGraph.getNodeGraph();
        auto& sorted = dag.getSorted();
        auto& layers = dag.getLayers();

        REQUIRE(sorted.size() == 5);
        REQUIRE(sorted[0] == 0);
        REQUIRE(sorted[1] == 1);
        REQUIRE(sorted[2] == 3);
        REQUIRE(sorted[3] == 2);
        REQUIRE(sorted[4] == 4);
        REQUIRE(layers.size() == 2);
        REQUIRE(layers[0].size() == 3);
        REQUIRE(layers[0][0] == 0);
        REQUIRE(layers[0][1] == 1);
        REQUIRE(layers[0][2] == 3);
        REQUIRE(layers[1].size() == 2);
        REQUIRE(layers[1][0] == 2);
        REQUIRE(layers[1][1] == 4);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Mixed Dependency", "[ecs::systemGraph]")
    {
        // Mix of explicit and implicit dependencies
        SystemGraph systemGraph;

        // To make it easier to keep track of this example here are some actual system names.
        // 0 = PhysicsSystem
        // 1 = MovementSystem
        // 2 = CameraFollowSystem
        // 3 = FallDamageSystem
        // 4 = AnimationStateSystem
        // 5 = NetworkReceiveSystem
        // 6 = NetworkSendSystem
        
        // Using the following components
        // 0 = Transform
        // 1 = Velocity

        // PhysicsSystem (0) reads Transform (0) writes Velocity (1)
        //     PhysicsSystem set to "First" placement
        // MovementSystem (1) read/writes Transform (0), reads Velocity (1)
        //     MovementSystem depends on PhysicsSystem (0)
        // CameraFollowSystem (2) reads Transform (0)
        //     CameraFollowSystem depends on MovementSystem (1)
        // FallDamageSystem (3) reads Transform (0), reads Velocity (1)
        //     FallDamageSystem depends on PhysicsSystem
        // AnimationStateSystem (4) reads Transform (0)
        // NetworkReceiveSystem (6) writes to Transform (0) writes to Velocity (1)
        //     NetworkReceiveSystem set to "First" placement
        // NetworkSendSystem (7) reads Transform (0) reads Velocity (1)
        //     NetworkSendSystem set to "Last" placement

        constexpr uint32_t PhysicsSystem = 0;
        constexpr uint32_t MovementSystem = 1;
        constexpr uint32_t CameraFollowSystem = 2;
        constexpr uint32_t FallDamageSystem = 3;
        constexpr uint32_t AnimationStateSystem = 4;
        constexpr uint32_t NetworkReceiveSystem = 5;
        constexpr uint32_t NetworkSendSystem = 6;

        constexpr uint32_t TransformComponent = 0;
        constexpr uint32_t VelocityComponent = 1;

        systemGraph.add(PhysicsSystem, { {TransformComponent, true}, {VelocityComponent, false} });
        systemGraph.add(MovementSystem, { {TransformComponent, false}, {VelocityComponent, true} });
        systemGraph.add(CameraFollowSystem, { {TransformComponent, true} });
        systemGraph.add(FallDamageSystem, { {TransformComponent, true}, {VelocityComponent, true} });
        systemGraph.add(AnimationStateSystem, { {TransformComponent, true} });
        systemGraph.add(NetworkReceiveSystem, { {TransformComponent, false}, {VelocityComponent, false} });
        systemGraph.add(NetworkSendSystem, { {TransformComponent, false}, {VelocityComponent, false} });

        systemGraph.addDependency(MovementSystem, PhysicsSystem);
        systemGraph.addDependency(CameraFollowSystem, MovementSystem);
        systemGraph.addDependency(FallDamageSystem, PhysicsSystem);

        systemGraph.setPlacementHint(PhysicsSystem, SystemNodePlacementHint::First);
        systemGraph.setPlacementHint(NetworkReceiveSystem, SystemNodePlacementHint::First);
        systemGraph.setPlacementHint(NetworkSendSystem, SystemNodePlacementHint::Last);

        REQUIRE(systemGraph.build() == true);

        auto& dag = systemGraph.getNodeGraph();
        auto& sorted = dag.getSorted();
        auto& layers = dag.getLayers();

        REQUIRE(sorted.size() == 7);
    } END_LITL_TEST_CASE
}