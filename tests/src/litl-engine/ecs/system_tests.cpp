#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "litl-engine/ecs/ecsCommon.hpp"
#include "litl-engine/ecs/world.hpp"
#include "litl-engine/ecs/component.hpp"
#include "litl-engine/ecs/system/system.hpp"
#include "litl-engine/ecs/system/systemBase.hpp"
#include "litl-engine/ecs/system/systemSchedule.hpp"

struct TestSystem
{
    using SystemComponents = LITL::Engine::ECS::SystemComponentList<Foo, Bar>;
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

TEST_CASE("System Test", "[engine::ecs::system]")
{
    // Option A: Extractor struct
    SystemComponentExtractor extractor;

    LITL::Engine::ECS::ExpandComponentList<TestSystem::SystemComponents>::apply(extractor);

    for (auto componentType : extractor.componentTypes)
    {
        std::cout << componentType->debugName << std::endl;
    }

    // Option B: Generic Lambda
    std::vector<LITL::Engine::ECS::ComponentDescriptor const*> descriptors;

    LITL::Engine::ECS::ExpandComponentList<TestSystem::SystemComponents>::apply(
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