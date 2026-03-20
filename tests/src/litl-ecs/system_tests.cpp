#include "tests.hpp"

#include "litl-ecs/common.hpp"
#include "litl-ecs/system/systemTraits.hpp"

namespace LITL::ECS::Tests
{
    struct TraitsTestSystem
    {
        void setup(Core::ServiceProvider& service) {};
        void update(World& world, float dt, Foo const& read, Bar& write) {}
    };

    /// <summary>
    /// Tests the internal ExpandSystemComponentList and SystemRunner by manually running the TestSystem.
    /// </summary>
    LITL_TEST_CASE("System Runner", "[ecs::system]")
    {
        LITL::ECS::World world;
        TestSystem system;

        world.addSystem<TestSystem>(SystemGroup::Update);

        auto entity0 = world.createImmediate();
        auto entity1 = world.createImmediate();

        world.addComponentsImmediate(entity0, Foo{ 0 }, Bar{ 0.0f, 0 });
        world.addComponentsImmediate(entity1, Foo{ 100 }, Bar{ 1.0f, 500 });

        auto entityRecord = world.getEntityRecord(entity0);

        SystemRunner<TestSystem> runner(&system);

        for (auto i = 0; i < 10; ++i)
        {
            runner.run(world, 0.0f, entityRecord.archetype->getChunk(entityRecord), entityRecord.archetype->chunkLayout());
        }

        REQUIRE(world.getComponent<Foo>(entity0)->a == 10);
        REQUIRE(LITL::Math::isZero(world.getComponent<Bar>(entity0)->a) == true);
        REQUIRE(world.getComponent<Bar>(entity0)->b == 10);

        REQUIRE(world.getComponent<Foo>(entity1)->a == 110);
        REQUIRE(LITL::Math::isOne(world.getComponent<Bar>(entity1)->a) == true);
        REQUIRE(world.getComponent<Bar>(entity1)->b == 510);

        world.destroyImmediate(entity0);
        world.destroyImmediate(entity1);
    } END_LITL_TEST_CASE

    void doSomething(ComponentTypeId id)
    {
        auto descriptor = ComponentDescriptor::get(id);
        int letmesee = 0;
    }

    LITL_TEST_CASE("Traits", "[ecs::system]")
    {
        using SystemComponentTuple = SystemComponents<TraitsTestSystem>;
        auto componentTypesTuple = SystemComponentsTupleOperations<SystemComponents<TraitsTestSystem>>::extractComponentIds();
        std::vector<ComponentTypeId> foundTypes;

        std::apply([&foundTypes](auto&&... componentTypes) {
            (foundTypes.push_back(componentTypes), ...);
        }, componentTypesTuple);

        // TraitsTestSystem::update(World&, float, Foo const&, Bar&)
        // Expect to see Foo and Bar (World and float are stripped out)

        REQUIRE(foundTypes.size() == 2);
        REQUIRE(foundTypes[0] == ComponentDescriptor::get<Foo>()->id);
        REQUIRE(foundTypes[1] == ComponentDescriptor::get<Bar>()->id);
    } END_LITL_TEST_CASE
}
