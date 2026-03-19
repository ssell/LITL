#include "tests.hpp"

#include "litl-core/services/serviceCollection.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/common.hpp"
#include "litl-ecs/world.hpp"
#include "litl-ecs/archetype/archetype.hpp"
#include "litl-ecs/archetype/archetypeRegistry.hpp"

namespace LITL::ECS::Tests
{
    class TestWorld : public World
    {
    public:

        SystemManager& extractSystemManager() noexcept
        {
            return getSystemManager();
        }
    };

    LITL_TEST_CASE("Empty Entity Creation and Destructon", "[ecs::world]")
    {
        World world;
        Entity entity = world.createImmediate();

        REQUIRE(world.isAlive(entity) == true);

        world.destroyImmediate(entity);

        REQUIRE(world.isAlive(entity) == false);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Add Many Entities", "[ecs::world]")
    {
        constexpr auto entityCount = 2048; // fill up multiple chunks worth

        World world;
        Archetype* fooBarArchetype = ArchetypeRegistry::get<Foo, Bar>();

        std::vector<Entity> entities;
        entities.reserve(entityCount);

        const auto initialFooBarCount = fooBarArchetype->entityCount();
        const auto initialEmptyCount = ArchetypeRegistry::Empty()->entityCount();

        for (auto i = 0; i < entityCount; ++i)
        {
            entities.push_back(world.createImmediate());
            world.addComponentsImmediate<Foo, Bar>(entities[i]);
        }

        REQUIRE(fooBarArchetype->entityCount() == (initialFooBarCount + entityCount));
        REQUIRE(ArchetypeRegistry::Empty()->entityCount() == initialEmptyCount);

        for (auto i = 0; i < entityCount; ++i)
        {
            world.destroyImmediate(entities[i]);
        }

        REQUIRE(fooBarArchetype->entityCount() == initialFooBarCount);
        REQUIRE(ArchetypeRegistry::Empty()->entityCount() == (initialEmptyCount + entityCount));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Entity Add/Remove Component", "[ecs::world]")
    {
        World world;
        Archetype* fooArchetype = ArchetypeRegistry::get<Foo>();
        Archetype* fooBarArchetype = ArchetypeRegistry::get<Foo, Bar>();
        Entity entity = world.createImmediate();

        const auto initialFooCount = fooArchetype->entityCount();
        const auto initialFooBarCount = fooBarArchetype->entityCount();

        // entity = [Foo]
        world.addComponentImmediate<Foo>(entity);

        REQUIRE(fooArchetype->entityCount() == (initialFooCount + 1));
        REQUIRE(fooBarArchetype->entityCount() == initialFooBarCount);
        REQUIRE(world.componentCount(entity) == 1);

        // entity = [Foo,Bar]
        world.addComponentImmediate<Bar>(entity);

        REQUIRE(fooArchetype->entityCount() == initialFooCount);
        REQUIRE(fooBarArchetype->entityCount() == (initialFooBarCount + 1));
        REQUIRE(world.componentCount(entity) == 2);

        // entity = [Foo,Bar]
        world.addComponentImmediate<Foo>(entity);

        REQUIRE(fooArchetype->entityCount() == initialFooCount);
        REQUIRE(fooBarArchetype->entityCount() == (initialFooBarCount + 1));
        REQUIRE(world.componentCount(entity) == 2);

        // entity = [Foo]
        world.removeComponentImmediate<Bar>(entity);

        REQUIRE(fooArchetype->entityCount() == (initialFooCount + 1));
        REQUIRE(fooBarArchetype->entityCount() == initialFooBarCount);
        REQUIRE(world.componentCount(entity) == 1);

        // entity = []
        world.removeComponentImmediate<Foo>(entity);

        REQUIRE(fooArchetype->entityCount() == initialFooCount);
        REQUIRE(fooBarArchetype->entityCount() == initialFooBarCount);
        REQUIRE(world.componentCount(entity) == 0);

        // entity = dead (but still in [])
        world.destroyImmediate(entity);

        REQUIRE(fooArchetype->entityCount() == initialFooCount);
        REQUIRE(fooBarArchetype->entityCount() == initialFooBarCount);
        REQUIRE(world.isAlive(entity) == false);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Entity Add / Remove Components", "[ecs::world]")
    {
        Archetype* fooBarArchetype = ArchetypeRegistry::get<Foo, Bar>();

        World world;
        Entity entity = world.createImmediate();

        const auto initialFooBarCount = fooBarArchetype->entityCount();

        world.addComponentsImmediate<Foo, Bar>(entity);

        REQUIRE(fooBarArchetype->entityCount() == (initialFooBarCount + 1));
        REQUIRE(world.componentCount(entity) == 2);

        world.addComponentsImmediate<Bar, Bar, Foo, Foo>(entity);

        REQUIRE(fooBarArchetype->entityCount() == (initialFooBarCount + 1));
        REQUIRE(world.componentCount(entity) == 2);

        world.removeComponentsImmediate<Foo, Bar>(entity);

        REQUIRE(fooBarArchetype->entityCount() == initialFooBarCount);
        REQUIRE(world.componentCount(entity) == 0);

        world.destroyImmediate(entity);

        REQUIRE(fooBarArchetype->entityCount() == initialFooBarCount);
        REQUIRE(world.isAlive(entity) == false);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Entity Add Components With Value", "[ecs::world]")
    {
        World world;
        Entity entity0 = world.createImmediate();
        Entity entity1 = world.createImmediate();

        world.addComponentImmediate(entity0, Foo{ 1337 });
        world.addComponentsImmediate(entity1, Foo{ 1338 }, Bar{ 10.0f, 100 });

        auto entity0Foo = world.getComponent<Foo>(entity0);
        auto entity1Foo = world.getComponent<Foo>(entity1);
        auto entity1Bar = world.getComponent<Bar>(entity1);

        REQUIRE(entity0Foo != std::nullopt);
        REQUIRE(entity1Foo != std::nullopt);
        REQUIRE(entity1Foo != std::nullopt);

        REQUIRE(entity0Foo->a == 1337);
        REQUIRE(entity1Foo->a == 1338);
        REQUIRE(entity1Bar->b == 100);
        REQUIRE(Math::fequals(entity1Bar->a, 10.0f));

        world.destroyImmediate(entity1);
        world.destroyImmediate(entity0);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Destroy Entity with Components", "[ecs::world]")
    {
        Archetype* fooBarArchetype = ArchetypeRegistry::get<Foo, Bar>();

        World world;
        Entity entity = world.createImmediate();
        const auto initialFooBarCount = fooBarArchetype->entityCount();

        world.addComponentsImmediate<Foo, Bar>(entity);

        REQUIRE(fooBarArchetype->entityCount() == (initialFooBarCount + 1));
        REQUIRE(world.componentCount(entity) == 2);

        world.destroyImmediate(entity);

        REQUIRE(fooBarArchetype->entityCount() == initialFooBarCount);
        REQUIRE(world.isAlive(entity) == false);
    } END_LITL_TEST_CASE

    namespace ComponentCtorDtorCallTest
    {
        static uint32_t CtorDtorTestCount = 0;

        struct CtorDtorTestComponent
        {
            CtorDtorTestComponent()
            {
                CtorDtorTestCount++;
            }

            ~CtorDtorTestComponent()
            {
                CtorDtorTestCount--;
            }

            uint32_t value;
        };
    }

    LITL_TEST_CASE("Component Ctor Dtor Call", "[ecs::world]")
    {
        World world;
        Entity entity0 = world.createImmediate();
        Entity entity1 = world.createImmediate();

        REQUIRE(ComponentCtorDtorCallTest::CtorDtorTestCount == 0);

        world.addComponentImmediate<ComponentCtorDtorCallTest::CtorDtorTestComponent>(entity0);

        REQUIRE(ComponentCtorDtorCallTest::CtorDtorTestCount == 1);

        world.addComponentImmediate<ComponentCtorDtorCallTest::CtorDtorTestComponent>(entity1);

        REQUIRE(ComponentCtorDtorCallTest::CtorDtorTestCount == 2);

        world.destroyImmediate(entity1);

        REQUIRE(ComponentCtorDtorCallTest::CtorDtorTestCount == 1);

        world.removeComponentImmediate<ComponentCtorDtorCallTest::CtorDtorTestComponent>(entity0);

        REQUIRE(ComponentCtorDtorCallTest::CtorDtorTestCount == 0);

        world.destroyImmediate(entity1);

        REQUIRE(ComponentCtorDtorCallTest::CtorDtorTestCount == 0);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Modify Component", "[ecs::world]")
    {
        World world;
        Entity entity = world.createImmediate();

        const uint32_t updatedValue = 55;

        world.addComponentImmediate<Foo>(entity);

        // Get
        auto fooOpt = world.getComponent<Foo>(entity);
        REQUIRE(fooOpt != std::nullopt);
        REQUIRE(fooOpt->a != updatedValue);

        // Set
        fooOpt->a = updatedValue;
        world.setComponent<Foo>(entity, *fooOpt);

        // Get again
        fooOpt = world.getComponent<Foo>(entity);
        REQUIRE(fooOpt != std::nullopt);
        REQUIRE(fooOpt->a == updatedValue);

        world.destroyImmediate(entity);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Modify Multiple Components", "[ecs::world]")
    {
        World world;
        Entity entity = world.createImmediate();

        world.addComponentsImmediate<Foo, Bar>(entity);

        Foo foo{ 50 };
        Bar bar{ 100.0f, 150 };

        world.setComponent<Foo>(entity, foo);
        world.setComponent<Bar>(entity, bar);

        std::optional<Foo> fooVal = world.getComponent<Foo>(entity);
        std::optional<Bar> barVal = world.getComponent<Bar>(entity);

        REQUIRE(fooVal != std::nullopt);
        REQUIRE(barVal != std::nullopt);

        REQUIRE(fooVal->a == foo.a);
        REQUIRE(Math::fequals(barVal->a, bar.a) == true);
        REQUIRE(barVal->b == bar.b);

        world.destroyImmediate(entity);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Modify Multiple Entity Components", "[ecs::world]")
    {
        World world;

        constexpr auto entityCount = 10;

        std::vector<Entity> entities;
        entities.reserve(entityCount);

        // Create entities
        for (auto i = 0; i < entityCount; ++i)
        {
            entities.push_back(world.createImmediate());
            world.addComponentsImmediate<Foo, Bar>(entities[i]);
        }

        Foo foo{};
        Bar bar{};

        // Update entities
        for (auto entity : entities)
        {
            world.setComponent<Foo>(entity, foo);
            world.setComponent<Bar>(entity, bar);

            foo.a++;
            bar.b++;
        }

        // Check updates went through
        for (auto i = 0; i < entityCount; ++i)
        {
            auto entity = entities[i];
            auto entityFoo = world.getComponent<Foo>(entity);
            auto entityBar = world.getComponent<Bar>(entity);

            REQUIRE(entityFoo->a == i);
            REQUIRE(entityBar->b == i);
        }

        for (auto entity : entities)
        {
            world.destroyImmediate(entity);
        }
    } END_LITL_TEST_CASE


    LITL_TEST_CASE("Run System", "[ecs::system]")
    {
        TestWorld world;

        auto entity0 = world.createImmediate();
        auto entity1 = world.createImmediate();

        world.addComponentsImmediate(entity0, Foo{ 0 }, Bar{ 0.0f, 0 });
        world.addComponentsImmediate(entity1, Foo{ 100 }, Bar{ 1.0f, 500 });

        /**
         * What is still needed:
         *
         *   - Advanced system scheduling
         *   - Fine to get started on main thread only to show it works, but quickly need to multithread
         */

        world.addSystem<TestSystem>(SystemGroup::Update);
        world.extractSystemManager().prepareFrame();

        for (auto i = 0; i < 10; ++i)
        {
            world.extractSystemManager().run(world, 0.0f, SystemGroup::Update);
        }

        REQUIRE(world.getComponent<Foo>(entity0)->a == 10);
        REQUIRE(Math::isZero(world.getComponent<Bar>(entity0)->a) == true);
        REQUIRE(world.getComponent<Bar>(entity0)->b == 10);

        REQUIRE(world.getComponent<Foo>(entity1)->a == 110);
        REQUIRE(Math::isOne(world.getComponent<Bar>(entity1)->a) == true);
        REQUIRE(world.getComponent<Bar>(entity1)->b == 510);

        world.destroyImmediate(entity0);
        world.destroyImmediate(entity1);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("System Setup", "[ecs::system]")
    {
        TestWorld world;
        Core::ServiceCollection collection{};
        collection.addSingleton<SystemSetupService>();

        auto services = collection.build();
        auto setupService = services->get<SystemSetupService>();

        REQUIRE(setupService != nullptr);
        REQUIRE(setupService->wasSetup == false);

        world.setup((*services));
        world.addSystem<TestSystem>(SystemGroup::Update);
        world.setupSystems((*services));

        REQUIRE(setupService->wasSetup == true);

    } END_LITL_TEST_CASE
}

REGISTER_TYPE_NAME(LITL::ECS::Tests::ComponentCtorDtorCallTest::CtorDtorTestComponent)