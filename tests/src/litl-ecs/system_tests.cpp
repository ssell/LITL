#include <catch2/catch_test_macros.hpp>
#include "litl-ecs/common.hpp"

/// <summary>
/// Tests the internal ExpandSystemComponentList and SystemRunner by manually running the TestSystem.
/// </summary>
TEST_CASE("System Runner", "[ecs::system]")
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