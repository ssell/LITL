#include <algorithm>
#include <array>
#include <vector>

#include "tests.hpp"
#include "litl-ecs/tests-common.hpp"
#include "litl-ecs/archetype/archetypeComponents.hpp"
#include "litl-ecs/archetype/chunkLayout.hpp"
#include "litl-core/hash.hpp"

namespace LITL::ECS::Tests
{
    LITL_TEST_CASE("Add", "[ecs::archetypeComponents]")
    {
        ArchetypeComponents components;

        REQUIRE(components.capacity() == Constants::max_components);
        REQUIRE(components.size() == 0);
        REQUIRE(components.dirty() == false);
        REQUIRE(components.hash() == 0);

        const auto fooId = getComponentTypeId<Foo>();
        const auto fooHash = Core::hash64(&fooId, sizeof(uint32_t));

        REQUIRE(components.add(fooId) == true);
        REQUIRE(components.capacity() == Constants::max_components);
        REQUIRE(components.size() == 1);
        REQUIRE(components.dirty() == true);

        REQUIRE(components.hash() == fooHash);
        REQUIRE(components.dirty() == false);

        REQUIRE(components.add(fooId) == true);         // duplicate, but ArchetypeComponents does not enforce uniqueness until hash
        REQUIRE(components.size() == 2);
        REQUIRE(components.dirty() == true);
        REQUIRE(components.hash() == fooHash);
        REQUIRE(components.dirty() == false);
        REQUIRE(components.size() == 1);                // check for dedupe

    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Remove", "[ecs::archetypeComponents]")
    {
        ArchetypeComponents components;

        const auto fooId = getComponentTypeId<Foo>();
        const auto fooHash = Core::hash64(&fooId, sizeof(uint32_t));

        REQUIRE(components.add(fooId) == true);
        REQUIRE(components.size() == 1);
        REQUIRE(components.hash() == fooHash);
        REQUIRE(components.dirty() == false);

        REQUIRE(components.remove(fooId) == true);
        REQUIRE(components.size() == 0);
        REQUIRE(components.capacity() == Constants::max_components);
        REQUIRE(components.dirty() == true);
        REQUIRE(components.hash() == 0);
        REQUIRE(components.dirty() == false);

        REQUIRE(components.remove(fooId) == false);     // nothing to remove
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Span", "[ecs::archetypeComponents]")
    {
        std::vector<ComponentTypeId> multiple{
            getComponentTypeId<Foo>(),
            getComponentTypeId<Foo>(),
            getComponentTypeId<Bar>(),
            getComponentTypeId<Baz>()
        };

        ArchetypeComponents components(multiple);

        REQUIRE(components.size() == 4);

        components.hash();

        REQUIRE(components.size() == 3);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Add Many", "[ecs::archetypeComponents]")
    {
        ArchetypeComponents components;

        std::vector<ComponentTypeId> multiple{
            getComponentTypeId<Foo>(),
            getComponentTypeId<Foo>(),
            getComponentTypeId<Bar>(),
            getComponentTypeId<Baz>()
        };

        REQUIRE(components.add(multiple) == true);
        REQUIRE(components.size() == 4);

        components.hash();

        REQUIRE(components.size() == 3);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Add Too Many", "[ecs::archetypeComponents]")
    {
        ArchetypeComponents components;
        std::vector<ComponentTypeId> tooMany;

        for (auto i = 0; i < components.capacity() + 1; ++i)
        {
            tooMany.push_back(getComponentTypeId<Foo>());
        }

        REQUIRE(components.add(tooMany) == false);
        REQUIRE(components.size() == 0);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Remove Many", "[ecs::archetypeComponents]")
    {
        std::vector<ComponentTypeId> multiple{
            getComponentTypeId<Foo>(),
            getComponentTypeId<Foo>(),
            getComponentTypeId<Bar>(),
            getComponentTypeId<Baz>()
        };

        ArchetypeComponents components(multiple);

        REQUIRE(components.size() == 4);

        std::vector<ComponentTypeId> toRemove{
            getComponentTypeId<Foo>(),
            getComponentTypeId<Bar>()
        };

        REQUIRE(components.remove(toRemove) == true);
        REQUIRE(components.size() == 1);        // leave only a single Baz

        components.hash();

        REQUIRE(components.size() == 1);        // no change
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Populate", "[ecs::archetypeComponents]")
    {
        ArchetypeComponents components;

        REQUIRE(components.size() == 0);

        ChunkLayout layout;
        layout.componentTypeCount = 3;
        layout.componentOrder[0] = ComponentDescriptor::get<Foo>();
        layout.componentOrder[1] = ComponentDescriptor::get<Bar>();
        layout.componentOrder[2] = ComponentDescriptor::get<Baz>();

        components.populate(&layout);

        REQUIRE(components.size() == 3);
        REQUIRE(components.dirty() == false);   // populate, unlike the other add methods, also performs a hash
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Hash", "[ecs::archetypeComponents]")
    {
        ArchetypeComponents components;

        std::array<ComponentTypeId, 3> array{ getComponentTypeId<Foo>(), getComponentTypeId<Bar>(), getComponentTypeId<Baz>() };
        std::sort(array.begin(), array.end());
        const auto expectedHash = Core::hashArray<ComponentTypeId>(array);

        REQUIRE(components.add(array) == true);
        REQUIRE(components.size() == 3);
        REQUIRE(components.hash() == expectedHash);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Component Ordering", "[ecs::archetypeComponents]")
    {
        ArchetypeComponents components;

        std::array<ComponentTypeId, 3> array{ getComponentTypeId<Baz>(), getComponentTypeId<Foo>(), getComponentTypeId<Bar>() };
        std::array<ComponentTypeId, 3> sorted = array;

        std::sort(sorted.begin(), sorted.end());

        REQUIRE(components.add(array) == true);
        REQUIRE(components.size() == 3);

        // prior to hash, retain original order
        REQUIRE(components[0] == array[0]);
        REQUIRE(components[1] == array[1]);
        REQUIRE(components[2] == array[2]);

        components.hash();

        // should now be sorted
        REQUIRE(components[0] == sorted[0]);
        REQUIRE(components[1] == sorted[1]);
        REQUIRE(components[2] == sorted[2]);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Copy", "[ecs::archetypeComponents]")
    {
        // Explicit copying is used internally by the ArchetypeRegistry, so lets make sure it always works.
        ArchetypeComponents a;
        ArchetypeComponents b;

        REQUIRE(a.size() == 0);
        REQUIRE(a.dirty() == false);

        REQUIRE(b.size() == 0);
        REQUIRE(b.dirty() == false);

        REQUIRE(a.add(getComponentTypeId<Foo>()) == true);
        REQUIRE(a.add(getComponentTypeId<Foo>()) == true);
        REQUIRE(a.add(getComponentTypeId<Bar>()) == true);

        REQUIRE(a.size() == 3);
        REQUIRE(a.dirty() == true);

        b = a;

        REQUIRE(b.size() == 3);
        REQUIRE(b.dirty() == true);

        a.hash();

        REQUIRE(a.size() == 2);
        REQUIRE(a.dirty() == false);
        REQUIRE(b.size() == 3);
        REQUIRE(b.dirty() == true);

        b = a;

        REQUIRE(b.size() == 2);
        REQUIRE(b.dirty() == false);

        REQUIRE(a.hash() == b.hash());

        REQUIRE(a[0] == b[0]);
        REQUIRE(a[1] == b[1]);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Hash Idempotency", "[ecs::archetypeCommands]")
    {
        ArchetypeComponents a;
        ArchetypeComponents b;

        a.add(getComponentTypeId<Foo>());
        a.add(getComponentTypeId<Bar>());
        a.add(getComponentTypeId<Baz>());

        b = a;
        const auto stable = b.hash();

        for (auto i = 0; i < 10; ++i)
        {
            REQUIRE(a.hash() == stable);
            REQUIRE(a.size() == 3);
            REQUIRE(a.dirty() == false);
        }
    } END_LITL_TEST_CASE
}