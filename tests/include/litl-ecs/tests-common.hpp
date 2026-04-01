#ifndef LITL_TESTS_ECS_COMMON_H__
#define LITL_TESTS_ECS_COMMON_H__

#include "litl-core/types.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-core/math/math.hpp"
#include "litl-ecs/world.hpp"
#include "litl-ecs/constants.hpp"

namespace LITL::ECS::Tests
{
    struct Foo
    {
        uint32_t a{ 0 };
    };

    struct Bar
    {
        float a{ 0.0f };
        uint32_t b{ 0 };
    };

    struct Baz
    {
        bool ok{ false };
    };

    struct SystemSetupService
    {
        bool wasSetup{ false };
    };

    struct TestSystem
    {
        void setup(Core::ServiceProvider& services)
        {
            auto setupService = services.get<SystemSetupService>();

            if (setupService != nullptr)
            {
                setupService->wasSetup = true;
            }
        }

        void update(ECS::EntityCommands& commands, float dt, Foo& foo, Bar& bar)
        {
            foo.a++;
            bar.b++;
        }
    };
}

REGISTER_TYPE_NAME(LITL::ECS::Tests::Foo)
REGISTER_TYPE_NAME(LITL::ECS::Tests::Bar)
REGISTER_TYPE_NAME(LITL::ECS::Tests::Baz)

#endif