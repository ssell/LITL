#include "tests.hpp"

#include "litl-core/services/serviceCollection.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/world.hpp"
#include "litl-ecs/system/systemRegistry.hpp"

#define BasicSystem(A) struct A { void setup(ServiceProvider& services) {} void update(EntityCommands& commands, float dt) {} };

namespace litl::tests
{
    namespace
    {
        BasicSystem(SIGSystemA);
        BasicSystem(SIGSystemB);
        BasicSystem(SIGSystemC);
        BasicSystem(SIGSystemD);
        BasicSystem(SIGSystemE);
    }

    LITL_TEST_CASE("Empty", "[ecs::systemInfoGraph]")
    {
        World world;
        SystemInfoGraph graph = world.buildInfoGraph();

        REQUIRE(graph.size() == 0);
    } LITL_END_TEST_CASE

    bool nodeEquals(SystemInfoNode const& node, SystemTypeId id, SystemGroup group, uint32_t layer)
    {
        return (node.id == id) && (node.group == group) && (node.layer == layer);
    }

    LITL_TEST_CASE("Simple", "[ecs::systemInfoGraph]")
    {
        World world;
        ServiceCollection serviceCollection; 
        ServiceProvider& serviceProvider = (*serviceCollection.build());
        SystemCollection& systemCollection = world.getSystemCollection();

        systemCollection.addSystem<SIGSystemA>(SystemGroup::Update);

        world.setupSystems(serviceProvider);
        SystemInfoGraph graph = world.buildInfoGraph();

        REQUIRE(graph.size() == 1);

        auto& nodes = graph.get();

        REQUIRE(nodes.size() == 1);
        REQUIRE(nodeEquals(nodes[0], SystemRegistry::getSystem<SIGSystemA>()->id(), SystemGroup::Update, 0));
    } LITL_END_TEST_CASE

        LITL_TEST_CASE("Multiple Dependencies", "[ecs::systemInfoGraph]")
    {
        World world;
        ServiceCollection serviceCollection;
        ServiceProvider& serviceProvider = (*serviceCollection.build());
        SystemCollection& systemCollection = world.getSystemCollection();

        systemCollection.addSystem<SIGSystemA>(SystemGroup::Update);
        systemCollection.addSystem<SIGSystemB>(SystemGroup::Update).placement(SystemPlacementHint::Last);
        systemCollection.addSystem<SIGSystemC>(SystemGroup::Update);
        systemCollection.addSystem<SIGSystemD>(SystemGroup::Update).placement(SystemPlacementHint::First);
        systemCollection.addSystem<SIGSystemE>(SystemGroup::Update).dependsOn<SIGSystemA>();;

        world.setupSystems(serviceProvider);

        // Expected layers:
        // [D]                  <--- wants first
        // [A, C]               <--- dont care
        // [E]                  <--- dependent on A
        // [B]                  <--- wants last

        SystemInfoGraph graph = world.buildInfoGraph();
        auto& nodes = graph.get();

        REQUIRE(nodes.size() == 5);
        REQUIRE(nodeEquals(nodes[0], SystemRegistry::getSystem<SIGSystemD>()->id(), SystemGroup::Update, 0));
        REQUIRE(nodeEquals(nodes[1], SystemRegistry::getSystem<SIGSystemA>()->id(), SystemGroup::Update, 1));
        REQUIRE(nodeEquals(nodes[2], SystemRegistry::getSystem<SIGSystemC>()->id(), SystemGroup::Update, 1));
        REQUIRE(nodeEquals(nodes[3], SystemRegistry::getSystem<SIGSystemE>()->id(), SystemGroup::Update, 2));
        REQUIRE(nodeEquals(nodes[4], SystemRegistry::getSystem<SIGSystemB>()->id(), SystemGroup::Update, 3));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Multiple Groups", "[ecs::systemInfoGraph]")
    {
        World world;
        ServiceCollection serviceCollection;
        ServiceProvider& serviceProvider = (*serviceCollection.build());
        SystemCollection& systemCollection = world.getSystemCollection();

        systemCollection.addSystem<SIGSystemA>(SystemGroup::Update);
        systemCollection.addSystem<SIGSystemB>(SystemGroup::Render).placement(SystemPlacementHint::Last);
        systemCollection.addSystem<SIGSystemC>(SystemGroup::FixedUpdate);
        systemCollection.addSystem<SIGSystemD>(SystemGroup::Startup).placement(SystemPlacementHint::First);
        systemCollection.addSystem<SIGSystemE>(SystemGroup::PreRender).dependsOn<SIGSystemA>();;

        world.setupSystems(serviceProvider);

        // Expected to just confirm to the group ordering.

        SystemInfoGraph graph = world.buildInfoGraph();
        auto& nodes = graph.get();

        REQUIRE(nodes.size() == 5);
        REQUIRE(nodeEquals(nodes[0], SystemRegistry::getSystem<SIGSystemD>()->id(), SystemGroup::Startup, 0));
        REQUIRE(nodeEquals(nodes[1], SystemRegistry::getSystem<SIGSystemC>()->id(), SystemGroup::FixedUpdate, 0));
        REQUIRE(nodeEquals(nodes[2], SystemRegistry::getSystem<SIGSystemA>()->id(), SystemGroup::Update, 0));
        REQUIRE(nodeEquals(nodes[3], SystemRegistry::getSystem<SIGSystemE>()->id(), SystemGroup::PreRender, 0));
        REQUIRE(nodeEquals(nodes[4], SystemRegistry::getSystem<SIGSystemB>()->id(), SystemGroup::Render, 0));
    } LITL_END_TEST_CASE
}