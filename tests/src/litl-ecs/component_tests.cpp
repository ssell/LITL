#include "tests.hpp"
#include "litl-core/hash.hpp"
#include "litl-core/containers/fixedSortedArray.hpp"
#include "litl-ecs/common.hpp"
#include "litl-ecs/component/component.hpp"
#include "litl-ecs/component/componentData.hpp"
#include "litl-ecs/component/componentRegistry.hpp"

namespace LITL::ECS::Tests
{
    LITL_TEST_CASE("Component Descriptor", "[ecs::component]")
    {
        auto fooDescriptor0 = ComponentDescriptor::get<Foo>();
        auto fooDescriptor1 = ComponentDescriptor::get<Foo>();
        auto barDescriptor0 = ComponentDescriptor::get<Bar>();
        auto barDescriptor1 = ComponentDescriptor::get<Bar>();

        REQUIRE(fooDescriptor0 == fooDescriptor1);
        REQUIRE(fooDescriptor0 != barDescriptor0);
        REQUIRE(barDescriptor0 == barDescriptor1);

        REQUIRE(fooDescriptor0->id == fooDescriptor1->id);
        REQUIRE(fooDescriptor0->id != barDescriptor0->id);
        REQUIRE(barDescriptor0->id == barDescriptor1->id);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("ComponentTypeId", "[ecs::component]")
    {
        auto fooDescriptor = ComponentDescriptor::get<Foo>();
        auto barDescriptor = ComponentDescriptor::get<Bar>();

        REQUIRE(getComponentTypeId<Foo>() == fooDescriptor->id);
        REQUIRE(getComponentTypeId<Bar>() == barDescriptor->id);
        REQUIRE(getComponentTypeId<Foo>() != getComponentTypeId<Bar>());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("StableComponentTypeId", "[ecs::component]")
    {
        auto fooDescriptor = ComponentDescriptor::get<Foo>();
        auto barDescriptor = ComponentDescriptor::get<Bar>();

        const auto expectedFooStableId = LITL::Core::hashString("LITL::ECS::Tests::Foo");
        const auto expectedBarStableId = LITL::Core::hashString("LITL::ECS::Tests::Bar");

        REQUIRE(fooDescriptor->stableId == expectedFooStableId);
        REQUIRE(getStableComponentTypeId<Foo>() == fooDescriptor->stableId);

        REQUIRE(barDescriptor->stableId == expectedBarStableId);
        REQUIRE(getStableComponentTypeId<Bar>() == barDescriptor->stableId);

        REQUIRE(getStableComponentTypeId<Foo>() != getStableComponentTypeId<Bar>());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("ComponentRegistry Tracking", "[ecs::component]")
    {
        auto fooDescriptor = ComponentDescriptor::get<Foo>();
        auto barDescriptor = ComponentDescriptor::get<Bar>();

        REQUIRE(ComponentRegistry::find(fooDescriptor->id) == fooDescriptor);
        REQUIRE(ComponentRegistry::findByStableId(fooDescriptor->stableId) == fooDescriptor);

        REQUIRE(ComponentRegistry::find(barDescriptor->id) == barDescriptor);
        REQUIRE(ComponentRegistry::findByStableId(barDescriptor->stableId) == barDescriptor);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("ComponentData Sort", "[ecs::componentData]")
    {

    } END_LITL_TEST_CASE
}