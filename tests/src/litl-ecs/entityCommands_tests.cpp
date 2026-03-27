#include "tests.hpp"

#include "litl-ecs/common.hpp"
#include "litl-ecs/entity/entityRegistry.hpp"
#include "litl-ecs/entity/entityCommands.hpp"
#include "litl-core/math/math.hpp"

namespace LITL::ECS::Tests
{
    LITL_TEST_CASE("Queue Single", "[ecs::entityCommands]")
    {
        EntityCommandQueue queue;

        REQUIRE(queue.count() == 0);
        REQUIRE(queue.empty() == true);

        queue.push(EntityCommand{ .type = EntityCommandType::DestroyEntity });

        REQUIRE(queue.count() == 1);
        REQUIRE(queue.empty() == false);

        auto command = queue.next();

        REQUIRE(command.has_value());
        REQUIRE(command.value().type == EntityCommandType::DestroyEntity);
        REQUIRE(command.value().queue == nullptr); // only for addcomponent
        REQUIRE(queue.count() == 0);
        REQUIRE(queue.empty() == true);

        command = queue.next();

        REQUIRE(!command.has_value());
        REQUIRE(queue.count() == 0);
        REQUIRE(queue.empty() == true);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Queue Component", "[ecs::entityCommands]")
    {
        EntityCommandQueue queue;

        Foo foo{ 50 };
        Bar bar{ 1337.0f, 8 };

        const auto fooDesc = ComponentDescriptor::get<Foo>();
        const auto barDesc = ComponentDescriptor::get<Bar>();

        queue.push(EntityCommand{
            .type = EntityCommandType::AddComponent,
            .component = fooDesc->id,
            }, &foo);

        queue.push(EntityCommand{
            .type = EntityCommandType::AddComponent,
            .component = barDesc->id,
            }, &bar);

        REQUIRE(queue.count() == 2);

        auto fooCommand = queue.next();
        auto barCommand = queue.next();

        REQUIRE(fooCommand.has_value());
        REQUIRE(barCommand.has_value());
        REQUIRE(queue.count() == 0);

        REQUIRE((*fooCommand).type == EntityCommandType::AddComponent);
        REQUIRE((*fooCommand).component == fooDesc->id);
        REQUIRE((*fooCommand).queue == &queue);

        REQUIRE((*barCommand).type == EntityCommandType::AddComponent);
        REQUIRE((*barCommand).component == barDesc->id);
        REQUIRE((*barCommand).queue == &queue);

        Foo fooCopy{};
        Bar barCopy{};

        REQUIRE(fooCopy.a == 0);
        REQUIRE(barCopy.a == 0);

        queue.loadComponent((*fooCommand), &fooCopy);
        queue.loadComponent((*barCommand), &barCopy);

        REQUIRE(fooCopy.a == foo.a);
        REQUIRE(Math::fequals(barCopy.a, bar.a));
        REQUIRE(barCopy.b == bar.b);

    } END_LITL_TEST_CASE

        LITL_TEST_CASE("Queue Many Components", "[ecs::entityCommands]")
    {
        // Tests adding a lot of components (enough to make 10 pool) and that they can be loaded.
        constexpr uint32_t commandCount = (Constants::entity_command_pool_size / sizeof(Foo)) * 10;

        EntityCommandQueue queue;
        Foo foo{ 0 };
        const auto fooType = ComponentDescriptor::get<Foo>()->id;

        // Run multiple times to make sure the recovery from all of the pops and resets is fine.
        for (auto i = 0; i < 10; ++i)
        {
            for (auto j = 0; j < commandCount; ++j)
            {
                foo.a = j;
                queue.push(EntityCommand{ .type = EntityCommandType::AddComponent, .component = fooType }, &foo);
            }

            bool isWrong = false;

            for (auto j = 0; j < commandCount && !isWrong; ++j)
            {
                auto command = queue.next();
                queue.loadComponent((*command), &foo);
                isWrong = foo.a != j;
            }

            REQUIRE(isWrong == false);
            REQUIRE(queue.empty());
            REQUIRE(queue.poolCount() == 10);

            queue.reset();

            // Pools should remain after reset, just their offsets reset to 0.
            REQUIRE(queue.poolCount() == 10);
        }
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Commands", "[ecs::entityCommands]")
    {
        EntityRegistry::clear();
        EntityCommands commands;
        const auto entityRecord = EntityRegistry::create();

        commands.destroyEntity(entityRecord.entity);
        commands.addComponent<Foo>(entityRecord.entity);
        commands.removeComponent<Foo>(entityRecord.entity);

        REQUIRE(commands.actionableCommandCount() == 3);
        EntityRegistry::clear();
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Materialize and Extract", "[ecs::entityCommands]")
    {
        EntityRegistry::clear();
        World world;
        EntityCommands commands;

        auto deferred0 = commands.createEntity();
        commands.addComponent<Foo>(deferred0);

        auto deferred1 = commands.createEntity();
        commands.addComponent<Bar>(deferred1);
        
        commands.removeComponent<Foo>(deferred0);
        commands.destroyEntity(deferred0);

        // Expected actionable commands:
        //      add component
        //      add component
        //      remove component
        //      destroy

        REQUIRE(commands.actionableCommandCount() == 4);

        std::vector<EntityCommand> materialized;
        materialized.resize(commands.actionableCommandCount());
        commands.extractCommands(&world, materialized, 0);

        REQUIRE(materialized[0].entity.isNull() == false);
        REQUIRE(materialized[0].type == EntityCommandType::AddComponent);
        REQUIRE(materialized[0].component == ComponentDescriptor::get<Foo>()->id);

        REQUIRE(materialized[1].entity.isNull() == false);
        REQUIRE(materialized[1].type == EntityCommandType::AddComponent);
        REQUIRE(materialized[1].component == ComponentDescriptor::get<Bar>()->id);

        REQUIRE(materialized[2].entity.isNull() == false);
        REQUIRE(materialized[2].type == EntityCommandType::RemoveComponent);
        REQUIRE(materialized[2].component == ComponentDescriptor::get<Foo>()->id);

        REQUIRE(materialized[3].entity.isNull() == false);
        REQUIRE(materialized[3].type == EntityCommandType::DestroyEntity);

        EntityRegistry::clear();
    } END_LITL_TEST_CASE
}