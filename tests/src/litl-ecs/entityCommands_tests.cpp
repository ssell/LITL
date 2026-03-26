#include "tests.hpp"

#include "litl-ecs/common.hpp"
#include "litl-ecs/entity/entityCommands.hpp"
#include "litl-core/math/math.hpp"

namespace LITL::ECS::Tests
{
    LITL_TEST_CASE("Queue Single", "[ecs::entityCommands]")
    {
        EntityCommandQueue queue;

        REQUIRE(queue.size() == 0);
        REQUIRE(queue.empty() == true);

        queue.push({ .type = EntityCommandType::CreateEntity });

        REQUIRE(queue.size() == 1);
        REQUIRE(queue.empty() == false);

        auto command = queue.pop();

        REQUIRE(command.has_value());
        REQUIRE(command.value().type == EntityCommandType::CreateEntity);
        REQUIRE(queue.size() == 0);
        REQUIRE(queue.empty() == true);

        command = queue.pop();

        REQUIRE(!command.has_value());
        REQUIRE(queue.size() == 0);
        REQUIRE(queue.empty() == true);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Queue Component", "[ecs::entityCommands]")
    {
        EntityCommandQueue queue;

        Foo foo{ 50 };
        Bar bar{ 1337.0f, 8 };

        const auto fooDesc = ComponentDescriptor::get<Foo>();
        const auto barDesc = ComponentDescriptor::get<Bar>();

        queue.push({
            .type = EntityCommandType::AddComponent,
            .component = fooDesc->id,
        }, &foo);

        queue.push({
            .type = EntityCommandType::AddComponent,
            .component = barDesc->id,
        }, &bar);

        REQUIRE(queue.size() == 2);

        auto fooCommand = queue.pop();
        auto barCommand = queue.pop();

        REQUIRE(fooCommand.has_value());
        REQUIRE(barCommand.has_value());
        REQUIRE(queue.size() == 0);

        REQUIRE((*fooCommand).type == EntityCommandType::AddComponent);
        REQUIRE((*fooCommand).component == fooDesc->id);

        REQUIRE((*barCommand).type == EntityCommandType::AddComponent);
        REQUIRE((*barCommand).component == barDesc->id);

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
                queue.push({ .type = EntityCommandType::AddComponent, .component = fooType }, &foo);
            }

            bool isWrong = false;

            for (auto j = 0; j < commandCount && !isWrong; ++j)
            {
                auto command = queue.pop();
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
}