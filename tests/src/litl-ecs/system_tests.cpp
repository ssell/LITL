#include <catch2/catch_test_macros.hpp>

#include "litl-ecs/common.hpp"
#include "litl-ecs/world.hpp"
#include "litl-ecs/system/system.hpp"

struct TestSystem
{
    void update(LITL::ECS::World& world, float dt, Foo& foo, Bar& bar)
    {
        foo.a++;
        bar.b++;
    }
};

/// <summary>
/// Tests the internal ExpandSystemComponentList and SystemRunner by manually running the TestSystem.
/// </summary>
TEST_CASE("System Runner", "[engine::ecs::system]")
{
    LITL::ECS::World world;
    TestSystem system;

    world.addSystem<TestSystem>(LITL::ECS::SystemGroup::Update);

    auto entity0 = world.createImmediate();
    auto entity1 = world.createImmediate();

    world.addComponentsImmediate(entity0, Foo{ 0 }, Bar{ 0.0f, 0 });
    world.addComponentsImmediate(entity1, Foo{ 100 }, Bar{ 1.0f, 500 });

    auto entityRecord = world.getEntityRecord(entity0);

    LITL::ECS::SystemRunner<TestSystem> runner(&system);

    for (auto i = 0; i < 10; ++i)
    {
        runner.run(world, 0.0f, entityRecord.archetype->getChunk(entityRecord), entityRecord.archetype->chunkLayout());
    }

    REQUIRE(world.getComponent<Foo>(entity0)->a == 10);
    REQUIRE(LITL::Math::isZero(world.getComponent<Bar>(entity0)->a) == true);
    REQUIRE(world.getComponent<Bar>(entity0)->b == 10);

    REQUIRE(world.getComponent<Foo>(entity1)->a == 110);
    REQUIRE(LITL::Math::isOne(world.getComponent<Bar>(entity1)->a) == true);
    REQUIRE(world.getComponent<Bar>(entity1)->b == 510);

    world.destroyImmediate(entity0);
    world.destroyImmediate(entity1);
}

TEST_CASE("System Wrapper", "[engine::ecs::system]")
{
    LITL::ECS::World world;

    auto entity0 = world.createImmediate();
    auto entity1 = world.createImmediate();

    world.addComponentsImmediate(entity0, Foo{ 0 }, Bar{ 0.0f, 0 });
    world.addComponentsImmediate(entity1, Foo{ 100 }, Bar{ 1.0f, 500 });

    auto entityRecord = world.getEntityRecord(entity0);

    LITL::ECS::System system;
    system.attach<TestSystem>();

    /**
     * What is still needed:
     * 
     *   - Match systems to archetypes
     *   - System needs to iterate its archetypes and chunks calling run
     *   - Calculate fixed dt and run fixed update multiple times per frame if needed
     *   - Advanced system scheduling
     *   - Fine to get started on main thread only to show it works, but quickly need to multithread
     */

    for (auto i = 0; i < 10; ++i)
    {
        system.run(world, 0.0f);
    }

    REQUIRE(world.getComponent<Foo>(entity0)->a == 10);
    REQUIRE(LITL::Math::isZero(world.getComponent<Bar>(entity0)->a) == true);
    REQUIRE(world.getComponent<Bar>(entity0)->b == 10);

    REQUIRE(world.getComponent<Foo>(entity1)->a == 110);
    REQUIRE(LITL::Math::isOne(world.getComponent<Bar>(entity1)->a) == true);
    REQUIRE(world.getComponent<Bar>(entity1)->b == 510);

    world.destroyImmediate(entity0);
    world.destroyImmediate(entity1);
}