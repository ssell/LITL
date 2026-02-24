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
        std::cout << "foo.a = " << foo.a++ << std::endl;
    }
};

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

TEST_CASE("System Extractor Test", "[engine::ecs::system]")
{
    // Option A: Extractor struct
    SystemComponentExtractor extractor;

    LITL::Engine::ECS::ExpandSystemComponentList<TestSystem::SystemComponents>::apply(extractor);

    for (auto componentType : extractor.componentTypes)
    {
        std::cout << componentType->debugName << std::endl;
    }

    // Option B: Generic Lambda
    std::vector<LITL::Engine::ECS::ComponentDescriptor const*> descriptors;

    LITL::Engine::ECS::ExpandSystemComponentList<TestSystem::SystemComponents>::apply(
        [&descriptors]<typename... ComponentTypes>()
        {
            extractComponentTypes<ComponentTypes...>(descriptors);
        }
    );

    for (auto componentType : descriptors)
    {
        std::cout << componentType->debugName << std::endl;
    }
}

TEST_CASE("System Runner Test", "[engine::ecs::system]")
{
    /*
    LITL::Engine::ECS::World world;

    auto entity0 = world.createImmediate();
    world.addComponentsImmediate<Foo, Bar>(entity0);

    auto entity1 = world.createImmediate();
    Foo foo{ 100 };
    world.addComponentsImmediate<Foo, Bar>(entity1);
    world.setComponent<Foo>(entity0, foo);

    auto entityRecord = world.getEntityRecord(entity0);

    TestSystem system;

    for (auto i = 0; i < 10; ++i)
    {
        LITL::Engine::ECS::ExpandSystemComponentList<TestSystem::SystemComponents>::apply(
            LITL::Engine::ECS::SystemRunner<TestSystem>{ system, * entityRecord.archetype, entityRecord.archetype->getChunk(entityRecord) });
    }

    world.destroyImmediate(entity0);
    world.destroyImmediate(entity1);
    */
}