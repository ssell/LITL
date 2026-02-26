#include <catch2/catch_test_macros.hpp>

#include "litl-core/hash.hpp"
#include "litl-ecs/common.hpp"
#include "litl-ecs/component/component.hpp"
#include "litl-ecs/component/componentRegistry.hpp"

TEST_CASE("Component Descriptor", "[ecs::component]")
{
    auto fooDescriptor0 = LITL::ECS::ComponentDescriptor::get<Foo>();
    auto fooDescriptor1 = LITL::ECS::ComponentDescriptor::get<Foo>();
    auto barDescriptor0 = LITL::ECS::ComponentDescriptor::get<Bar>();
    auto barDescriptor1 = LITL::ECS::ComponentDescriptor::get<Bar>();

    REQUIRE(fooDescriptor0 == fooDescriptor1);
    REQUIRE(fooDescriptor0 != barDescriptor0);
    REQUIRE(barDescriptor0 == barDescriptor1);

    REQUIRE(fooDescriptor0->id == fooDescriptor1->id);
    REQUIRE(fooDescriptor0->id != barDescriptor0->id);
    REQUIRE(barDescriptor0->id == barDescriptor1->id);
}

TEST_CASE("ComponentTypeId", "[ecs::component]")
{
    auto fooDescriptor = LITL::ECS::ComponentDescriptor::get<Foo>();
    auto barDescriptor = LITL::ECS::ComponentDescriptor::get<Bar>();

    REQUIRE(LITL::ECS::getComponentTypeId<Foo>() == fooDescriptor->id);
    REQUIRE(LITL::ECS::getComponentTypeId<Bar>() == barDescriptor->id);
    REQUIRE(LITL::ECS::getComponentTypeId<Foo>() != LITL::ECS::getComponentTypeId<Bar>());
}

TEST_CASE("StableComponentTypeId", "[ecs::component]")
{
    auto fooDescriptor = LITL::ECS::ComponentDescriptor::get<Foo>();
    auto barDescriptor = LITL::ECS::ComponentDescriptor::get<Bar>();

    const auto expectedFooStableId = LITL::Core::hashString("struct Foo");
    const auto expectedBarStableId = LITL::Core::hashString("struct Bar");

    REQUIRE(fooDescriptor->stableId == expectedFooStableId);
    REQUIRE(LITL::ECS::getStableComponentTypeId<Foo>() == fooDescriptor->stableId);

    REQUIRE(barDescriptor->stableId == expectedBarStableId);
    REQUIRE(LITL::ECS::getStableComponentTypeId<Bar>() == barDescriptor->stableId);

    REQUIRE(LITL::ECS::getStableComponentTypeId<Foo>() != LITL::ECS::getStableComponentTypeId<Bar>());
}

TEST_CASE("ComponentRegistry Tracking", "[ecs::component]")
{
    auto fooDescriptor = LITL::ECS::ComponentDescriptor::get<Foo>();
    auto barDescriptor = LITL::ECS::ComponentDescriptor::get<Bar>();

    REQUIRE(LITL::ECS::ComponentRegistry::find(fooDescriptor->id) == fooDescriptor);
    REQUIRE(LITL::ECS::ComponentRegistry::findByStableId(fooDescriptor->stableId) == fooDescriptor);

    REQUIRE(LITL::ECS::ComponentRegistry::find(barDescriptor->id) == barDescriptor);
    REQUIRE(LITL::ECS::ComponentRegistry::findByStableId(barDescriptor->stableId) == barDescriptor);
}