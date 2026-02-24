#include <catch2/catch_test_macros.hpp>

#include "litl-core/hash.hpp"
#include "litl-engine/ecs/ecsCommon.hpp"
#include "litl-engine/ecs/component.hpp"
#include "litl-engine/ecs/componentRegistry.hpp"

TEST_CASE("Component Descriptor", "[engine::ecs::component]")
{
    auto fooDescriptor0 = LITL::Engine::ECS::ComponentDescriptor::get<Foo>();
    auto fooDescriptor1 = LITL::Engine::ECS::ComponentDescriptor::get<Foo>();
    auto barDescriptor0 = LITL::Engine::ECS::ComponentDescriptor::get<Bar>();
    auto barDescriptor1 = LITL::Engine::ECS::ComponentDescriptor::get<Bar>();

    REQUIRE(fooDescriptor0 == fooDescriptor1);
    REQUIRE(fooDescriptor0 != barDescriptor0);
    REQUIRE(barDescriptor0 == barDescriptor1);

    REQUIRE(fooDescriptor0->id == fooDescriptor1->id);
    REQUIRE(fooDescriptor0->id != barDescriptor0->id);
    REQUIRE(barDescriptor0->id == barDescriptor1->id);
}

TEST_CASE("ComponentTypeId", "[engine::ecs::component]")
{
    auto fooDescriptor = LITL::Engine::ECS::ComponentDescriptor::get<Foo>();
    auto barDescriptor = LITL::Engine::ECS::ComponentDescriptor::get<Bar>();

    REQUIRE(LITL::Engine::ECS::getComponentTypeId<Foo>() == fooDescriptor->id);
    REQUIRE(LITL::Engine::ECS::getComponentTypeId<Bar>() == barDescriptor->id);
    REQUIRE(LITL::Engine::ECS::getComponentTypeId<Foo>() != LITL::Engine::ECS::getComponentTypeId<Bar>());
}

TEST_CASE("StableComponentTypeId", "[engine::ecs::component]")
{
    auto fooDescriptor = LITL::Engine::ECS::ComponentDescriptor::get<Foo>();
    auto barDescriptor = LITL::Engine::ECS::ComponentDescriptor::get<Bar>();

    const auto expectedFooStableId = LITL::Core::hashString("struct Foo");
    const auto expectedBarStableId = LITL::Core::hashString("struct Bar");

    REQUIRE(fooDescriptor->stableId == expectedFooStableId);
    REQUIRE(LITL::Engine::ECS::getStableComponentTypeId<Foo>() == fooDescriptor->stableId);

    REQUIRE(barDescriptor->stableId == expectedBarStableId);
    REQUIRE(LITL::Engine::ECS::getStableComponentTypeId<Bar>() == barDescriptor->stableId);

    REQUIRE(LITL::Engine::ECS::getStableComponentTypeId<Foo>() != LITL::Engine::ECS::getStableComponentTypeId<Bar>());
}

TEST_CASE("ComponentRegistry Tracking", "[engine::ecs::component]")
{
    auto fooDescriptor = LITL::Engine::ECS::ComponentDescriptor::get<Foo>();
    auto barDescriptor = LITL::Engine::ECS::ComponentDescriptor::get<Bar>();

    REQUIRE(LITL::Engine::ECS::ComponentRegistry::find(fooDescriptor->id) == fooDescriptor);
    REQUIRE(LITL::Engine::ECS::ComponentRegistry::findByStableId(fooDescriptor->stableId) == fooDescriptor);

    REQUIRE(LITL::Engine::ECS::ComponentRegistry::find(barDescriptor->id) == barDescriptor);
    REQUIRE(LITL::Engine::ECS::ComponentRegistry::findByStableId(barDescriptor->stableId) == barDescriptor);
}