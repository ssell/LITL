#include "tests.hpp"
#include "litl-core/hash.hpp"
#include "litl-core/containers/fixedSortedArray.hpp"
#include "litl-ecs/tests-common.hpp"
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
        const auto fooId = getComponentTypeId<Foo>();
        const auto barId = getComponentTypeId<Bar>();
        const auto bazId = getComponentTypeId<Baz>();

        // FixedSortedArray is a crucial part of World::mutate
        Core::FixedSortedArray<ComponentData> components({
            ComponentData{ bazId, nullptr },
            ComponentData{ fooId, nullptr },
            ComponentData{ barId, nullptr },
            ComponentData{ fooId, nullptr },
            ComponentData{ barId, nullptr },
            ComponentData{ barId, nullptr }
        });

        // Do not know the sorting at compile time as it can be influenced by other tests
        // since component id is given on a first-come first-serve basis.
        std::array<ComponentTypeId, 3> expectedResult = { fooId, barId, bazId };
        std::sort(expectedResult.begin(), expectedResult.end());

        REQUIRE(components.size() == 3);
        REQUIRE((components.begin() + 0)->type == expectedResult[0]);
        REQUIRE((components.begin() + 1)->type == expectedResult[1]);
        REQUIRE((components.begin() + 2)->type == expectedResult[2]);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("ComponentTypeId Conversion", "[ecs::componentData]")
    {
        const auto fooId = getComponentTypeId<Foo>();
        const auto barId = getComponentTypeId<Bar>();
        const auto bazId = getComponentTypeId<Baz>();

        ComponentData fooData{ fooId, nullptr };
        ComponentData barData{ barId, nullptr };
        ComponentData bazData{ bazId, nullptr };

        ComponentTypeId foo = fooData;
        ComponentTypeId bar = barData;
        ComponentTypeId baz = bazData;

        REQUIRE(foo == fooId);
        REQUIRE(bar == barId);
        REQUIRE(baz == bazId);
    } END_LITL_TEST_CASE
}