#include <catch2/catch_test_macros.hpp>
#include "litl-engine/ecs/component.hpp"

struct Foo
{
    uint32_t a;
};

struct Bar
{
    float a;
    uint32_t b;
};

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