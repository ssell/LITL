#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "litl-engine/ecs/ecsCommon.hpp"
#include "litl-engine/ecs/world.hpp"
#include "litl-engine/ecs/component.hpp"
#include "litl-engine/ecs/system/system.hpp"

struct TestSystem
{
    using SystemComponents = LITL::Engine::ECS::SystemComponentList<Foo, Bar>;

    void update(Foo& foo, Bar& bar)
    {
        foo.a++;
        bar.b++;
    }
};

/// <summary>
/// Tests the internal SystemRunner by manually running the TestSystem.
/// </summary>
TEST_CASE("System Runner Test", "[engine::ecs::system]")
{
    LITL::Engine::ECS::World world;
    TestSystem system;

    auto entity0 = world.createImmediate();
    auto entity1 = world.createImmediate();

    world.addComponentsImmediate(entity0, Foo{ 0 }, Bar{ 0.0f, 0 });
    world.addComponentsImmediate(entity1, Foo{ 100 }, Bar{ 1.0f, 500 });

    auto entityRecord = world.getEntityRecord(entity0);

    for (auto i = 0; i < 10; ++i)
    {
        LITL::Engine::ECS::ExpandSystemComponentList<TestSystem::SystemComponents>::apply(
            LITL::Engine::ECS::SystemRunner<TestSystem>{ system, * entityRecord.archetype, entityRecord.archetype->getChunk(entityRecord) });
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

template<typename ComponentType>
void extractComponentType(std::vector<LITL::Engine::ECS::ComponentDescriptor const*>& componentTypes)
{
    componentTypes.push_back(LITL::Engine::ECS::ComponentDescriptor::get<ComponentType>());
}

template<typename... ComponentTypes>
void extractComponentTypes(std::vector<LITL::Engine::ECS::ComponentDescriptor const*>& componentTypes)
{
    componentTypes.reserve(sizeof... (ComponentTypes));
    (extractComponentType<ComponentTypes>(componentTypes), ...);
}

struct SystemComponentExtractor
{
    std::vector<LITL::Engine::ECS::ComponentDescriptor const*> componentTypes;

    template<typename... ComponentTypes>
    void operator()()
    {
        extractComponentTypes<ComponentTypes...>(componentTypes);
    }
};